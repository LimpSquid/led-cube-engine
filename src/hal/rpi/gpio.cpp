#include <hal/rpi/gpio.hpp>
#include <filesystem>
#include <cstdio>
#include <chrono>
#include <thread>

using namespace std::chrono;
namespace fs = std::filesystem;

namespace
{

std::string const base_dir = "/sys/class/gpio";
std::string const pin_dir = base_dir + "/gpio";
std::string const export_filepath = base_dir + "/export";
std::string const unexport_filepath = base_dir + "/unexport";

std::FILE * fopen_or_throw(std::string const & filepath, char const * mode)
{
    auto f = std::fopen(filepath.c_str(), mode);
    auto const deadline = system_clock::now() + 100ms; // Because it takes a while for the `direction` and `value` files to be openable after a pin is exported
    while (!f && system_clock::now() < deadline) {
        std::this_thread::sleep_for(5ms);
        f = std::fopen(filepath.c_str(), mode);
    }
    if (!f)
        throw std::runtime_error("Failed to open file '" + filepath + "'");
    return f;
}

void fwrite_or_throw(std::string const & filepath, std::string const & out)
{
    auto f = fopen_or_throw(filepath, "r+");
    std::size_t const size = std::fwrite(out.c_str(), 1, out.size(), f);
    std::fclose(f);
    if (size != out.size())
        throw std::runtime_error("Failed to write to file '" + filepath + "'");
}

void fread_or_throw(std::string const & filepath, std::string & buf)
{
    auto f = fopen_or_throw(filepath, "r");
    std::size_t const size = std::fread(buf.data(), 1, buf.size(), f);
    std::fclose(f);
    if (size == 0)
        throw std::runtime_error("Failed to read from file '" + filepath + "'");
    buf.resize(size);
}

bool export_gpio(unsigned int pin)
{
    std::string const pin_str = std::to_string(pin);

    if (fs::exists(pin_dir + pin_str))
        return false; // Already exported

    fwrite_or_throw(export_filepath, pin_str);
    return true;
}

} // End of namespace

namespace hal::rpi
{

gpio::gpio(unsigned int pin, direction dir) :
    pin_(pin),
    exported_(export_gpio(pin))
{
    try {
        fwrite_or_throw(pin_dir + std::to_string(pin) + "/direction", dir == output ? "out" : "in");
    } catch (...) {
        unexport();
        throw;
    }
}

gpio::gpio(gpio && other) :
    pin_(other.pin_),
    exported_(other.exported_)
{
    other.exported_ = false;
}

gpio::~gpio()
{
    unexport();
}

gpio::level gpio::read() const
{
    std::string buf;
    buf.resize(8);

    fread_or_throw(pin_dir + std::to_string(pin_) + "/value", buf);
    if (buf == "0")
        return lo;
    if (buf == "1")
        return hi;
    throw std::runtime_error("Invalid GPIO level: " + buf);
}

void gpio::write(level lvl) const
{
    fwrite_or_throw(pin_dir + std::to_string(pin_) + "/value", std::to_string(static_cast<int>(lvl)));
}

void gpio::unexport() const
{
    if (exported_)
        fwrite_or_throw(unexport_filepath, std::to_string(pin_));
}

} // End of namespace
