#include "server.h"
#include "gui.h"

int main(int argc, char *argv[]) {

    Process example;
    example.command = "/usr/bin/bash";
    example.arguments = {"run.sh"};
//    example.arguments = {"-jar", "/home/anis/Documents/BRUH/libraries/net/minecraftforge/forge/1.20.1-47.4.23/forge-1.20.1-47.4.23-server.jar"};
    example.workingDirectory = "/home/anis/Documents/BRUH";
    
    executeProgram(example);
    getSyncOutput(example);
    return 0;
}