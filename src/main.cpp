#include <cube/process/entry.hpp>
#include <functional>
#include <iostream>
#include <map>

using namespace cube::process;

namespace
{

char const * const default_proc = "render";
std::map<std::string, std::function<int(int, char const * const [])>> const procs =
{
    {"render", main_render},
    {"library", main_library}
};

void exit_help()
{
    std::cout
        << "Usage: led-cube-engine <process> [arg...]\n\n"
        << "Available processes:\n";

    for (auto const & proc : procs)
        std::cout << "  - " << proc.first << '\n';
    std::exit(EXIT_FAILURE);
}

} // End of namespace

int main(int ac, char const * const av[])
{
    auto const search = ac >= 2 ? procs.find(av[1]) : procs.find(default_proc);
    if (search == procs.cend())
        exit_help();

    try {
        return search->second(ac, av);
    } catch (std::exception const & ex) {
        std::cerr << "Application exited with error: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
