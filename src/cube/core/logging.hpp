#pragma once

#include <utility>
#include <string>
#include <chrono>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <unistd.h>

#define LOG_ARG(name, value)    std::make_pair(name, value)
#define LOG(level, ...)         cube::core::log(STDOUT_FILENO, level, __VA_ARGS__)
#define LOG_INF(...)            LOG(cube::core::log_prio::info, __VA_ARGS__)
#define LOG_DBG(...)            LOG(cube::core::log_prio::debug, __VA_ARGS__)
#define LOG_WRN(...)            LOG(cube::core::log_prio::warning, __VA_ARGS__)
#define LOG_ERR(...)            LOG(cube::core::log_prio::error, __VA_ARGS__)
#define LOG_PLAIN(...)          LOG(cube::core::log_prio::plain, __VA_ARGS__)
#define LOG_INF_IF(expr, ...)   do { if (expr) { LOG_INF(__VA_ARGS__); } } while(0)
#define LOG_DBG_IF(expr, ...)   do { if (expr) { LOG_DBG(__VA_ARGS__); } } while(0)
#define LOG_WRN_IF(expr, ...)   do { if (expr) { LOG_WRN(__VA_ARGS__); } } while(0)
#define LOG_ERR_IF(expr, ...)   do { if (expr) { LOG_ERR(__VA_ARGS__); } } while(0)

// Name conflicts shouldn't be a problem as described by
// section "block scope" in https://en.cppreference.com/w/cpp/language/scope
#define LOG_PERIODIC(level, min_interval, ...)          \
    do {                                                \
        using namespace std::chrono;                    \
        thread_local steady_clock::time_point previous; \
        auto now = steady_clock::now();                 \
        if (now - previous >= min_interval) {           \
            LOG(level, __VA_ARGS__);                    \
            previous = std::move(now);                  \
        }                                               \
    } while (0)
#define LOG_INF_PERIODIC(min_interval, ...) LOG_PERIODIC(cube::core::log_prio::info, min_interval, __VA_ARGS__)
#define LOG_DBG_PERIODIC(min_interval, ...) LOG_PERIODIC(cube::core::log_prio::debug, min_interval, __VA_ARGS__)
#define LOG_WRN_PERIODIC(min_interval, ...) LOG_PERIODIC(cube::core::log_prio::warning, min_interval, __VA_ARGS__)
#define LOG_ERR_PERIODIC(min_interval, ...) LOG_PERIODIC(cube::core::log_prio::error, min_interval, __VA_ARGS__)

namespace cube::core
{

enum class log_prio
{
    error = 0,
    warning,
    info,
    debug,

    plain = -1, // No special formatting except the arguments
};

inline log_prio get_runtime_log_level()
{
    using std::operator""s;

    char const * const debug = std::getenv("DEBUG");
    char const * const quiet = std::getenv("QUIET");

    if (debug && debug == "1"s)
        return log_prio::debug;
    else if(quiet && quiet == "1"s)
        return log_prio::warning;
    else
        return log_prio::info;
}

template<typename T>
using log_arg = std::pair<char const *, T>;

inline std::size_t write(char * buffer, std::string_view str)
{
    while (!str.empty() && str.front() == '\n')
        str.remove_prefix(1);
    while (!str.empty() && str.back() == '\n')
        str.remove_suffix(1);
    std::copy(str.begin(), str.end(), buffer);
    return str.size();
}

inline std::size_t write(char * buffer, char const * const str)
{
    return write(buffer, std::string_view(str));
}

inline std::size_t write(char * buffer, bool value)
{
    buffer[0] = value ? 't' : 'f';
    return 1;
}

template<typename T>
struct as_hex
{
    as_hex(T a) :
        arg(a)
    { }

    T arg;
    static_assert(std::is_integral_v<T>);
};

template<typename T>
std::size_t write(char * buffer, as_hex<T> const & value)
{
    return sprintf(buffer, "0x%lx", static_cast<unsigned long>(value.arg));
}

template<typename T>
typename std::enable_if_t<std::is_integral_v<T>, std::size_t> write(char * buffer, T const & value)
{
    if constexpr (std::is_signed_v<T>)
        return sprintf(buffer, "%li", static_cast<long>(value));
    else
        return sprintf(buffer, "%lu", static_cast<unsigned long>(value));
}

template<typename T>
typename std::enable_if_t<std::is_floating_point_v<T>, std::size_t> write(char * buffer, T const & value)
{
    return sprintf(buffer, "%f", value);
}

template<class Rep, std::intmax_t Num, std::intmax_t Den>
std::size_t write(char * buffer, std::chrono::duration<Rep, std::ratio<Num, Den>> const & value)
{
    using ratio_t = std::ratio<Num, Den>;

    if constexpr (std::is_same_v<ratio_t, std::nano>)
        return sprintf(buffer, "%ldns", value.count());
    else if constexpr (std::is_same_v<ratio_t, std::micro>)
        return sprintf(buffer, "%ldus", value.count());
    else if constexpr (std::is_same_v<ratio_t, std::milli>)
        return sprintf(buffer, "%ldms", value.count());
    else if constexpr (std::is_same_v<ratio_t, std::ratio<1>>)
        return sprintf(buffer, "%lds", value.count());
    else if constexpr (std::is_same_v<ratio_t, std::ratio<60>>)
        return sprintf(buffer, "%ld minute(s)", value.count());
    else if constexpr (std::is_same_v<ratio_t, std::ratio<3600>>)
        return sprintf(buffer, "%ld hour(s)", value.count());
    else if constexpr (std::is_same_v<ratio_t, std::ratio<86400>>)
        return sprintf(buffer, "%ld day(s)", value.count());
    else if constexpr (std::is_same_v<ratio_t, std::ratio<604800>>)
        return sprintf(buffer, "%ld week(s)", value.count());
    else if constexpr (std::is_same_v<ratio_t, std::ratio<2629746>>)
        return sprintf(buffer, "%ld month(s)", value.count());
    else if constexpr (std::is_same_v<ratio_t, std::ratio<31556952>>)
        return sprintf(buffer, "%ld year(s)", value.count());
    else
        return sprintf(buffer, "%ld [num: %ld, den: %ld]", value.count(), Num, Den);
}

template<typename T>
std::size_t write(char * buffer, log_arg<T> const & arg)
{
    std::size_t off = write(buffer, "\033[0;97m");
    off += write(buffer + off, arg.first);
    off += write(buffer + off, "\033[0m");
    off += write(buffer + off, "=");
    off += write(buffer + off, arg.second);
    return off;
}

template<typename T, typename ... A>
std::size_t write(char * buffer, log_arg<T> const & arg, log_arg<A> const & ... args)
{
    std::size_t off = write(buffer, arg);
    if constexpr (sizeof ... (args)) {
        off += write(buffer + off, ", ");
        off += write(buffer + off, args ...);
    }
    return off;
}

template<typename ... T>
void log(int fd, log_prio prio, std::string_view msg, log_arg<T> ... args)
{
    // Console colors: https://gist.github.com/jneander/7443174
    static std::string_view const prefix[] = { "ERRO: ", "WARN: ", "INFO: ", "DEBG: "};
    static std::string_view const color[]  = { "31",     "34",     "32",     "33"    };
    static const log_prio log_level = get_runtime_log_level();

    if (prio > log_level)
        return;

    thread_local char buffer[2048]; // Todo: handle out of bounds array access?
    thread_local std::time_t time;
    std::size_t off = 0;

    if (prio != log_prio::plain) {
        time = std::time(nullptr);
        off += write(buffer + off, std::asctime(std::localtime(&time)));
        off += write(buffer + off, " \033[1;");
        off += write(buffer + off, color[static_cast<std::size_t>(prio)]);
        off += write(buffer + off, "m");
        off += write(buffer + off, prefix[static_cast<std::size_t>(prio)]);
        off += write(buffer + off, "\033[0m");
    }

    off += write(buffer + off, msg);
    if constexpr (sizeof ... (args)) {
        off += write(buffer + off, " (");
        off += write(buffer + off, args ...);
        off += write(buffer + off, ") ");
    }
    buffer[off++] = '\n';

    auto result = ::write(fd, buffer, off);
    assert(result != -1);
}

} // End of namespace
