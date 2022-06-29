#include "server/WebServer.h"
#include <iostream>

using namespace std;

int main() {
    // WebServer server(1316, 3, 500, true, "localhost", 3306, "root", "password", "testdb", 12, 1000, 6, true, 1, 1024);
    WebServer server;
    server.start();
    return 0;
}