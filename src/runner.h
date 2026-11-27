#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <curl/curl.h>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include <boost/process/v1.hpp>
#include <boost/process/v1/io.hpp>
#include <boost/process/v1/pipe.hpp>
#include <thread>
#include <Ultralight/Ultralight.h>
#include <Ultralight/JS.h>
#include <AppCore/App.h>
#include <AppCore/Window.h>
namespace bp = boost::process::v1;
using namespace std;


struct Process {
    string command;
    vector<string> data, arguments;
    string workingDirectory;
    bp::ipstream is, err;
    bp::opstream os;
    bp::child proc;
};

void executeProgram(Process& program);
void killProgram(Process& program);
bool getOutput(Process& program);
bool getError(Process& program);
bool getSyncOutput(Process& program);
bool giveInput(Process& program, const string& command);
bool isRunning(Process& program);
