#include "server.h"

Server::Server(ServerProperties &p) {
    _properties = p;
}

bool Server::start() {
    _process = executeCommand("java", {"-jar", "server.jar"}, QString::fromStdString(_properties.folderPath));
    return true;
}

bool Server::stop() {
    if (_process && _process->state() == QProcess::Running) {
        sendCommand("stop");
        return true;
    } else {
        cerr << "ERROR: Failed to stop server!\n";
        return false;
    }
}

bool Server::createEnv() {
    if (createNewInstance()) {
        // Set EULA to true
        fstream file(_properties.folderPath + "/eula.txt", ios::out);
        file << "eula=true\n";
        return true;
    } else {
        cerr << "ERROR: Failed to get a new instance!\n";
        return false;
    }
}

bool Server::sendCommand(const QString& cmd) {
    if (_process && _process->state() == QProcess::Running) {
        _process->write((cmd + "\n").toUtf8());
        return true;
    } else {
        cerr << "ERROR: There is no running server!\n";
        return false;
    }
}

size_t write_data(void* ptr, size_t size, size_t nmemb, std::fstream* stream) {
    size_t totalSize = size * nmemb;
    stream->write(static_cast<char*>(ptr), totalSize);
    return totalSize;
}

bool Server::getServerFile() {
    CURL *curl;
    fstream fp;
    
    CURLcode res;
    char *url = "https://fill-data.papermc.io/v1/objects/a7116085a8ba88cf00771d5e61a2925007942092650811da7327dc23243f0cec/paper-26.3-255.jar";
    string output = _properties.folderPath + "/server.jar";
    curl = curl_easy_init();

    if (curl) {
        fp.open(output, ios::out | ios::binary);
        if (fp.is_open()) {
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fp);
            res = curl_easy_perform(curl);

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
    if (!fs::create_directory(_properties.folderPath)) {
        cout << "NOTICE: Failed to crete a new directory, it already exists!\n";
    }

    // Download required server file
    if (!fs::exists(_properties.folderPath + "/server.jar") && getServerFile()) {
        cout << "NOTICE: Successfully downloaded the file!\n";
        return true;
    } else if (fs::exists(_properties.folderPath + "/server.jar")){
        cout << "NOTICE: A server file already exists!\n";
        return true;
    } else {
        cerr << "ERROR: Couldn't download/load a server file!\n";
        return false;
    }

}
