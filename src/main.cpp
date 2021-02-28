#include <iostream>
#include <cstdlib>
#include <rest/server.h>

int main(int argc, char *argv[])
{
    Rest::Server srv("127.0.0.1", "8080");

    return EXIT_SUCCESS;
}
