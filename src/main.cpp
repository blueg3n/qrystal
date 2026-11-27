#include "server.h"
#include "gui.h"

int main() {
    ServerProperties prop = {"Test1", "1.21.1", "paper"};

    Server server(prop);
    server.create();
    
    thread t1(&Server::start, &server);
    thread t2(render);
    t1.join();
    t2.join();
    return 0;
}