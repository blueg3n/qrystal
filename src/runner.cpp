#include "runner.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
bool executeCommand(sting c) { 
    return false;
}
#else
vector<string> executeCommand(string c) {
//    system(c.c_str());
    vector<string> data;
    char buffer[2048];
    FILE *newFile = popen(c.c_str(), "r");
    if (newFile) {
        if (fgets(buffer, 2048, newFile) != nullptr) {
                data.push_back(buffer);            
        }
        pclose(newFile);
    }
    return data;
}

void outputData(vector<string> d) {
    for (string s : d) {
        cout << s << "\n";
    }
}
#endif