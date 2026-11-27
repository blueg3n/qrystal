#pragma once

#include "runner.h"
#include <filesystem>
using namespace nlohmann;
namespace fs = filesystem;


struct ServerProperties {
    string serverName;
    string serverVersion;
    string serverType;
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
    void stop();
    bool create();
    bool sendCommand(const string& command);

private:
    bool getServerFile(string version, string type);
    bool createNewInstance();
    bool createScript();
    void addServerJSON();
    void getServerJSON();

    string _serverTypeStr;
    ServerProperties _properties;
    Process proc;
    json data;
};
