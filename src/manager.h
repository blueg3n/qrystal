#pragma once

#include <sstream>
using namespace std;

enum ServerType {
    CLASSIC,
    FABRIC,
    NEOFORGE,
    FORGE,
    PAPER
};

struct ServerProperties {
    string serverName;
    int serverVersion;
    ServerType serverType;
    bool state;
    int minAllocRAM = 2;
    int maxAllocRam = 2;    
    string folderPath;
};

class Server {
public:
    Server(ServerProperties &properties);
    bool start();
    bool stop();

private:
    ServerType type;
    ServerProperties properties;
};