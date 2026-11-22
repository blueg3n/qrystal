#pragma once

#include "runner.h"
#include <filesystem>
using namespace std;
using namespace nlohmann;
namespace fs = filesystem;

enum ServerType {
    CLASSIC,
    FABRIC,
    NEOFORGE,
    FORGE,
    PAPER
};

struct ServerProperties {
    string serverName;
    string serverVersion;
    string folderPath;
    ServerType serverType;
    int id;
    bool state;
    int minAllocRAM = 2;
    int maxAllocRAM = 2;    
};

class Server {
public:
    // Create a new server
    Server(ServerProperties &properties);
    
    // Import existing one
    Server(string serverName);
    bool start();
    bool stop();
    bool create();
    bool sendCommand(const string& command);

private:
    bool getServerFile(string version, string type);
    bool createNewInstance();
    bool loadInstance(string folderPath);
    bool createScript();
    void addServerJSON();
    void getServerJSON();

    string getType();
    string _serverTypeStr;
    ServerProperties _properties;
    json data;
};
