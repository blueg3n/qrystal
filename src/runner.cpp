#include "runner.h"

bp::child initChild(string cmd, bp::ipstream& out, bp::ipstream& err, bp::opstream& in, const string& wDir, const vector<string>& args) {
    return bp::child(cmd, bp::std_out > out, bp::std_err > err, bp::std_in < in, bp::start_dir(wDir), bp::args(args));
}

void executeProgram(Process& prg) {
    prg.proc = initChild(prg.command, prg.is, prg.err, prg.os, prg.workingDirectory, prg.arguments);
}

bool getOutput(Process& prg) {
    string isl = "";
    if (!isRunning(prg)) {
        cerr << "ERROR: There is no current active process\n";
        return false;
    }

    while (isRunning(prg) && getline(prg.is, isl)) {
        prg.data.push_back(isl);
        cout << isl << "\n";
    } 

    return true;
}

bool getError(Process& prg) {
    // NOTICE: If true, there means there is an error. Don't mix it up
    
    string isl = "";
    vector<string> temp;
    if (!isRunning(prg)) {
        cerr << "ERROR: There is no current active process\n";
        return true;
    }

    while (isRunning(prg) && getline(prg.err, isl)) {
        temp.push_back(isl);
    }

    if (!temp.empty()) {
        for (string e : temp) cout << e << "\n";
        return true;
    }

    return false;
}

// NOTICE: Syncs Output and Error pipelines so you don't have to write them seperately
bool getSyncOutput(Process &prg) {
    thread out(getOutput, ref(prg));
    thread err(getError, ref(prg));
    out.join();
    err.join();
    return true;
}

bool giveInput(Process& prg, const string& cmd) {
    if (isRunning(prg)) {
        prg.os << cmd << "\n";
        prg.os.flush();
    } else {
        cerr << "ERROR: There is no current active process\n";
        return false;
    }

    return true;
}

bool isRunning(Process& prg) {
    return prg.proc.running();
}