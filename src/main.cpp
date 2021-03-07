#include <iostream>
#include <rest/http/httpserver.h>

using namespace Rest::Http;

int main(int argc, char *argv[])
{
    HttpServer srv("127.0.0.1", "50000");

    srv.begin();
    srv.run();
    srv.end();

    return EXIT_SUCCESS;
}
