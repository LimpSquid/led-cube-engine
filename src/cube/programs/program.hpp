#pragma once

#include <string>
#include <functional>

namespace cube::programs
{

using program_entry_t = std::function<int(int, char const * const [])>;
using program_sigint_t = std::function<void()>;

struct program_definition
{
    std::string name;
    std::string desc;

    program_entry_t entry;
    program_sigint_t sigint;
};

class program_registry
{
public:
    static program_registry & instance();
    std::vector<program_definition> const & programs() const;
    void register_program(program_definition defintion);

private:
    program_registry() = default;
    program_registry(program_registry &) = delete;
    program_registry(program_registry &&) = delete;

    std::vector<program_definition> programs_;
};

struct program
{
    program(std::string name, std::string desc, program_entry_t entry, program_sigint_t sigint);
    program(std::string name, program_entry_t entry);
    program(std::string name, program_entry_t entry, program_sigint_t sigint);
    program(std::string name, std::string desc, program_entry_t entry);
};

} // End of namespace
