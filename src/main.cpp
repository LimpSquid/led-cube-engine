#include <cube/programs/entry.hpp>
#include <functional>
#include <iostream>
#include <map>

using namespace cube::programs;

namespace
{

struct program
{
    std::function<int(int, char const * const [])> entry;
    std::string desc;
};

std::map<std::string, program> const programs =
{
    {"render",  {main_render,   "render animations to the LED cube engine's graphics device"}},
    {"library", {main_library,  "list info about the LED cube engine's animation library"}},
};

void exit_with_help()
{
    std::cout
        << "Usage: led-cube-engine <program> [arg...]\n\n"
        << "Available programs:\n";

    for (auto const & prog : programs)
        std::cout << "  - " << prog.first << "\t\t" << prog.second.desc << '\n';
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
        return search->second.entry(ac - 1, &av[1]);
    } catch (std::exception const & ex) {
        std::cerr << "Application exited with error: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
