#include "server.h"

Server::Server(ServerProperties &p) {
    _properties = p;
    proc.command = "/usr/bin/bash";
    proc.arguments = {"run.sh"};
    proc.workingDirectory = "servers/" + _properties.serverName;
}

Server::Server(string sN) {
    _properties.serverName = sN;

    getServerJSON();
}

bool Server::start() {
    if (!fs::exists("servers/" + _properties.serverName + "/server.jar")) {
        cerr << "ERROR: server.jar doesn't exist!\n";
        return false;
    }

    if (!fs::exists("servers/" + _properties.serverName + "/run.sh")) {
        if (!createScript()) {
            cerr << "ERROR: Couldn't create the bash script!\n";
            return false;
        }
    }

    // TODO: Add execution
    executeProgram(proc);
    getSyncOutput(proc);

    return true;
}

void Server::stop() {
    // TODO: Stop the server
    killProgram(proc);
}

bool Server::create() {
    if (createNewInstance()) {
        // Set EULA to true
        fstream file("servers/" + _properties.serverName + "/eula.txt", ios::out);
        file << "eula=true\n";
        return true;
    } else {
        cerr << "ERROR: Failed to get a new instance!\n";
        return false;
    }
}

bool Server::sendCommand(const string& cmd) {
    // TODO: Be able to send commands to server instance
    return giveInput(proc, cmd);
}

size_t write_data(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t totalSize = size * nmemb;
    auto* stream = static_cast<fstream*>(userdata);
    stream->write(ptr, totalSize);
    return stream->good() ? totalSize : 0;
}

bool Server::getServerFile(string ver, string type) {
    CURL *curl;
    fstream fp;

    CURLcode res;
    string url = "https://mcutils.com/api/server-jars/" + type + "/" + ver + "/download";
    string output = "servers/" + _properties.serverName + "/server.jar";
    curl = curl_easy_init();

    if (curl) {
        fp.open(output, ios::out | ios::binary);
        if (fp.is_open()) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fp);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                cerr << "Download failed: " << curl_easy_strerror(res) << '\n';

                curl_easy_cleanup(curl);
                fp.close();

                fs::remove(output);
                return false;
            }

            curl_easy_cleanup(curl);
            fp.close();

        } else {
            cerr << "ERROR: Failed to save the server file!\n";
            return false;
        }

    } else {
        cerr << "ERROR: Failed to load CURL!\n";
        return false;
    }

    return true;
}

bool Server::createNewInstance() {
    // Create Folder
    std::error_code ec;

    fs::create_directories("servers/" + _properties.serverName, ec);

    if (ec) {
        cerr << "Directory creation failed: " << ec.message() << '\n';
        return false;
    }

    // Download required server file
    if (!fs::exists("servers/" + _properties.serverName + "/server.jar") && getServerFile(_properties.serverVersion, _properties.serverType)) {
        cout << "NOTICE: Successfully downloaded the file!\n";
    } else if (fs::exists("servers/" + _properties.serverName + "/server.jar")){
        cout << "NOTICE: A server file already exists!\n";
    } else {
        cerr << "ERROR: Couldn't download/load a server file!\n";
        return false;
    }

    addServerJSON();

    return true;
}

bool Server::createScript() {
    ofstream script("servers/" + _properties.serverName + "/run.sh");
    
    if (!script.is_open()) return false;
    
    script << "java -Xmx" + to_string(_properties.maxAllocRAM) + "G" + " -Xms" +
    to_string(_properties.minAllocRAM) + "G" + " -jar server.jar\n";
    
    script.close();
    
    return true;
}

void Server::addServerJSON() {

    data["servers"][_properties.serverName]["version"] = _properties.serverVersion;
    data["servers"][_properties.serverName]["type"] = _properties.serverType;
    ofstream serverList("serverList.json");

    serverList << data.dump(4);
}

void Server::getServerJSON() {
    ifstream serverList("serverList.json");
    data = json::parse(serverList);

    _properties.serverVersion = data["servers"][_properties.serverName]["version"];
    _properties.serverType = data["servers"][_properties.serverName]["type"];
}

