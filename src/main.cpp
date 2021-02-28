#include <iostream>
#include <rest/server.h>

int main(int argc, char *argv[])
{
    Rest::Server srv("127.0.0.1", "50000");

    srv.begin();
    srv.run();
    srv.end();

    return EXIT_SUCCESS;
}
