#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include "runner.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QLabel wsp("Hello World!");

    vector<string> data = executeCommand("ls -l");
    outputData(data);

    wsp.show();
    return app.exec();
 }