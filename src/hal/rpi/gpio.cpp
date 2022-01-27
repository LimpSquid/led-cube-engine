#include <hal/rpi/gpio.hpp>
#include <filesystem>
#include <cstdio>

namespace fs = std::filesystem;

namespace
{

std::string const base_dir = "/sys/class/gpio";
std::string const pin_dir = base_dir + "/gpio";
std::string const export_filepath = base_dir + "/export";
std::string const unexport_filepath = base_dir + "/unexport";

bool export_gpio(unsigned int pin)
{
    std::string const pin_str = std::to_string(pin);

    if (fs::exists(pin_dir + pin_str))
        return false; // Already exported

    auto f = std::fopen(export_filepath.c_str(), "r+");
    if (!f)
        throw std::runtime_error("Failed to open GPIO export file for pin: " + pin_str);

    std::size_t const size = std::fwrite(pin_str.c_str(), 1, pin_str.size(), f);
    if (size != pin_str.size())
        throw std::runtime_error("Failed to export GPIO for pin: " + pin_str);
    return true;
}

} // End of namespace

namespace hal::rpi
{

gpio::gpio(unsigned int pin, direction dir) :
    pin_(pin),
    exported_(export_gpio(pin))
{

}

gpio::~gpio()
{

}

} // End of namespace
