#include <cube/programs/program.hpp>
#include <cube/core/logging.hpp>
#include <cube/core/utils.hpp>
#include <iostream>
#include <vector>
#include <signal.h>
#include <execinfo.h>

using namespace cube::core;
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

void handle_segfault()
{
    void * entries[64];
    int const size = backtrace(entries, 64);

    std::cerr << "\n================ BACKTRACE ================\n\n";
    backtrace_symbols_fd(entries, size, STDERR_FILENO);
    std::cerr << "\n";

    // Assign default handler and forward segfault signal
    signal(SIGSEGV, SIG_DFL);
    kill(getpid(), SIGSEGV);
}

void signal_handler(int signal)
{
    if (signal == SIGINT)
        return visit(prog->sigint);
    if (signal == SIGSEGV)
        return handle_segfault();

    throw std::runtime_error("Unhandled signal: "s + std::to_string(signal));
}

} // End of namespace

int main(int ac, char const * const av[])
{
    if (ac < 2)
        exit_with_help();

    auto const & programs = program_registry::instance().programs();
    auto const search = std::find_if(programs.begin(), programs.end(),
        [arg=std::string_view(av[1])](auto const & p) { return p.name == arg && p.entry; });
    if (search == programs.cend())
        exit_with_help();
    prog = *search;

    signal(SIGINT, signal_handler);
    signal(SIGSEGV, signal_handler);

    try {
        return prog->entry(ac - 1, &av[1]);
    } catch (std::exception const & ex) {
        LOG_ERR("Application exited with error", LOG_ARG("what", ex.what()));
        return EXIT_FAILURE;
    }
}
