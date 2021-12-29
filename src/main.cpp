#include <cube/programs/entry.hpp>
#include <functional>
#include <iostream>
#include <map>

using namespace cube::programs;

namespace
{

char const * const default_program = "render";
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
    auto const search = ac >= 2 ? programs.find(av[1]) : programs.find(default_program);
    if (search == programs.cend())
        exit_with_help();

    try {
        return search->second(ac - 1, &av[1]);
    } catch (std::exception const & ex) {
        std::cerr << "Application exited with error: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
