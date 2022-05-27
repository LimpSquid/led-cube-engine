#pragma once

#include <type_traits>
#include <cstdint>

namespace hal::rpi_cube
{

template<typename T, T Polynomial, T Seed>
class basic_crc_generator
{
public:
    using value_type = T;
    using polynomial = std::integral_constant<value_type, Polynomial>;
    using seed = std::integral_constant<value_type, Seed>;

    basic_crc_generator & operator()(void const * src, std::size_t size)
    {
        bool b;
        unsigned char const * data = reinterpret_cast<unsigned char const *>(src);

        for (unsigned int i = 0; i < size; ++i) {
            crc_ = static_cast<value_type>(crc_ ^ *data);
            for (unsigned int j = 0; j < 8; ++j) {
                b = (crc_ & 0x1);
                crc_ = static_cast<value_type>(crc_ >> 1);
                if (b)
                    crc_ ^= polynomial::value;
            }
            ++data;
        }
        return *this;
    }

    operator value_type() const
    {
        return crc_;
    }

    value_type get() const
    {
        return value_type{*this};
    }

    void reset()
    {
        crc_ = seed::value;
    }

private:
    static_assert(std::is_unsigned_v<value_type>);

    value_type crc_{seed::value};
};

using crc16_generator = basic_crc_generator<uint16_t, 0x1021, 0xffff>; // CRC-16-CCITT-FALSE

} // End of namespace
