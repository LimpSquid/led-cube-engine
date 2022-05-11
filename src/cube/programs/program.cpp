#include <cube/programs/program.hpp>

namespace cube::programs
{

program_registry & program_registry::instance()
{
    static program_registry instance;
    return instance;
}

std::vector<program_definition> const & program_registry::programs() const
{
    return programs_;
}

void program_registry::register_program(program_definition defintion)
{
    programs_.push_back(std::move(defintion));
}

program::program(std::string name, std::string desc, program_entry_t entry, program_sigint_t sigint)
{
    program_registry::instance().register_program({std::move(name), std::move(desc), std::move(entry), std::move(sigint)});
}

program::program(std::string name, program_entry_t entry) :
    program(std::move(name), "n.a.", std::move(entry), nullptr)
{ }

program::program(std::string name, program_entry_t entry, program_sigint_t sigint) :
    program(std::move(name), "n.a.", std::move(entry), std::move(sigint))
{ }

program::program(std::string name, std::string desc, program_entry_t entry) :
    program(std::move(name), std::move(desc), std::move(entry), nullptr)
{ }

} // End of namespace
