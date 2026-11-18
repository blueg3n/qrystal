#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    ServerProperties prop = {"Test", "26.3", "example", PAPER};

    Server server(prop);

    server.createEnv();
    server.start();

    return app.exec();
}