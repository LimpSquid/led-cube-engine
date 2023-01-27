#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <optional>
#include <algorithm>
#include <csignal>
#include <unistd.h>

namespace cube::core
{

class safe_fd
{
public:
    safe_fd() : fd(-1) { }
    safe_fd(int descriptor) : fd(descriptor) { }
    safe_fd(safe_fd && other) : fd(other.fd) { other.fd = -1; }
    ~safe_fd() { reset(); }

    void reset(int descriptor = -1)
    {
        // Close old fd before assigning new one
        if (fd >= 0)
            ::close(fd);

        fd = descriptor;
    }

    void swap(safe_fd & other) { std::swap(fd, other.fd); }
    operator int() const { return fd; }
    safe_fd & operator=(int descriptor) { reset(descriptor); return *this; }

private:
    int fd;
};

using scope_guard_t = std::shared_ptr<void *>;
inline scope_guard_t make_scope_guard() { return std::make_shared<void *>(nullptr); }
inline scope_guard_t::weak_type get_ref(scope_guard_t tracker) { return {tracker}; }
inline bool lock(scope_guard_t::weak_type weak) { return weak.lock() != nullptr; }

inline void throw_errno(char const * const op = nullptr)
{
    using std::operator""s;

    if (op)
        throw std::runtime_error("Failed operation '"s + op + "': " + std::strerror(errno));
    else
        throw std::runtime_error("Error: "s + std::strerror(errno));
}

template<typename ... A>
struct signature
{
    template<typename T, typename R>
    constexpr static auto select_overload(R (T::*f)(A ...)) { return f; }
    template<typename R>
    constexpr static auto select_overload(R (*f)(A ...)) { return f; }
};

inline bool question_yesno(const char * message)
{
    std::string answer;
    auto const check_answer = [&]() {
        std::transform(answer.begin(), answer.end(), answer.begin(),
            [](unsigned char x) { return std::tolower(x); });

        bool ok = answer == "y"
            || answer == "n"
            || answer == "yes"
            || answer == "no";

        return ok
            ? answer[0] == 'y'
            : std::optional<bool>{};
    };

    auto const handler = signal(SIGINT, SIG_DFL);
    if (handler == SIG_ERR)
        throw std::runtime_error("Failed installing default SIGINT handler");

    std::cout << message << "? [Y/n]\n";

    std::optional<bool> result;
    for (;;) {
        std::cin >> answer;
        result = check_answer();
        if (result)
            break;
        std::cout
            << "Invalid answer: " << answer << " Please try again\n"
            << message << "? [Y/n]\n";
    }

    if (signal(SIGINT, handler) == SIG_ERR)
        throw std::runtime_error("Failed installing SIGINT handler");

    if (!std::cin)
        throw std::runtime_error("Reading user input failed");
    return *result;
}

} // End of namespace
