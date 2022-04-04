#include <cube/programs/entry.hpp>
#include <functional>
#include <iostream>
#include <map>
#include <signal.h>

using namespace cube::programs;
using std::operator""s;

namespace
{

struct program
{
    std::function<int(int, char const * const [])> entry;
    std::function<void()> sigint;
    std::string desc;
};

std::map<std::string, program> const programs =
{
    {"render",  {main_render,   sigint_render,  "render animations to the LED cube engine's graphics device"}},
    {"library", {main_library,  sigint_library, "list info about the LED cube engine's animation library"}},
};

std::optional<program> prog;

void exit_with_help()
{
    std::cout
        << "Usage: led-cube-engine <program> [arg...]\n\n"
        << "Available programs:\n";

    for (auto const & prog : programs)
        std::cout << "  - " << prog.first << "\t\t" << prog.second.desc << '\n';
    std::exit(EXIT_FAILURE);
}

void signal_handler(int signal)
{
    if (signal == SIGINT)
        return prog->sigint();

    throw std::runtime_error("Unhandled signal: "s + std::to_string(signal));
}

} // End of namespace

int main(int ac, char const * const av[])
{
    if (ac < 2)
        exit_with_help();
    auto const search = programs.find(av[1]);
    if (search == programs.cend())
        exit_with_help();
    prog = search->second;

    signal(SIGINT, signal_handler);

    try {
        return prog->entry(ac - 1, &av[1]);
    } catch (std::exception const & ex) {
        std::cerr << "Application exited with error: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
