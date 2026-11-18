#pragma once

#include "runner.h"
#include <filesystem>
using namespace std;
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
    bool state;
    int minAllocRAM = 2;
    int maxAllocRAM = 2;    
};

class Server {
public:
    Server(ServerProperties &properties);
    bool start();
    bool stop();
    bool createEnv();
    bool sendCommand(const QString& command);

private:
    bool getServerFile();
    bool createNewInstance();
    ServerType _type;
    ServerProperties _properties;
    QProcess* _process = nullptr;
};
