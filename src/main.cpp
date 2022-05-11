#include <cube/programs/program.hpp>
#include <cube/core/logging.hpp>
#include <iostream>
#include <vector>
#include <signal.h>

using namespace cube::programs;
using std::operator""s;

namespace
{

std::optional<program_definition> prog;

void exit_with_help()
{
    std::cout
        << "Usage: led-cube-engine <program> [arg...]\n\n"
        << "Available programs:\n";

    for (auto const & prog : program_registry::instance().programs())
        std::cout << "  - " << prog.name << "\t\t" << prog.desc << '\n';
    std::exit(EXIT_FAILURE);
}

void signal_handler(int signal)
{
    auto exec = [](auto handler) {
        if (handler)
            handler();
    };

    if (signal == SIGINT)
        return exec(prog->sigint);
    throw std::runtime_error("Unhandled signal: "s + std::to_string(signal));
}

} // End of namespace

int main(int ac, char const * const av[])
{
    if (ac < 2)
        exit_with_help();

    auto const & programs = program_registry::instance().programs();
    auto const search = std::find_if(programs.begin(), programs.end(),
        [arg=std::string_view(av[1])](auto const & p) { return p.name == arg; });
    if (search == programs.cend())
        exit_with_help();
    prog = *search;

    signal(SIGINT, signal_handler);

    try {
        return prog->entry(ac - 1, &av[1]);
    } catch (std::exception const & ex) {
        LOG_ERR("Application exited with error", LOG_ARG("what", ex.what()));
        return EXIT_FAILURE;
    }
}
