#include <cube/programs/entry.hpp>
#include <functional>
#include <iostream>
#include <map>

using namespace cube::programs;

namespace
{

std::map<std::string, std::function<int(int, char const * const [])>> const programs =
{
    {"render", main_render},
    {"library", main_library}
};

void exit_with_help()
{
    std::cout
        << "Usage: led-cube-engine <program> [arg...]\n\n"
        << "Available programs:\n";

    for (auto const & prog : programs)
        std::cout << "  - " << prog.first << '\n';
    std::exit(EXIT_FAILURE);
}

} // End of namespace

int main(int ac, char const * const av[])
{
    if (ac < 2)
        exit_with_help();
    auto const search = programs.find(av[1]);
    if (search == programs.cend())
        exit_with_help();

    try {
        return search->second(ac - 1, &av[1]);
    } catch (std::exception const & ex) {
        std::cerr << "Application exited with error: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
