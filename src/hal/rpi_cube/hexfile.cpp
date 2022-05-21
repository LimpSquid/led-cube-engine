#include <hal/rpi_cube/hexfile.hpp>
#include <cube/core/utils.hpp>
#include <fstream>
#include <charconv>
#include <cassert>

using namespace cube::core;
using std::operator""s;
namespace fs = std::filesystem;

namespace
{

constexpr std::size_t minimum_line_size = 11; // Start code (1) + byte count (2) + address (4) + record type (2) + checksum (2)

enum class data_field
{
    byte_count,
    address,
    record_type,
    extended_address,
};

enum class record_type
{
    data                        = 0,
    eof                         = 1,
    extended_segment_address    = 2,
    start_segment_address       = 3,
    extended_linear_address     = 4,
    start_linear_address        = 5,
};

std::string_view data_view(std::string_view line)
{
    line.remove_prefix(9); // Start code, byte count, address and record type
    line.remove_suffix(2); // Checksum
    return line;
}

template<data_field F>
expected_or_error<uint32_t> parse(std::string_view line)
{
    uint32_t out = 0;
    std::from_chars_result result;

    if constexpr (F == data_field::byte_count)
        result = std::from_chars(line.data() + 1, line.data() + 3, out, 16);
    else if constexpr (F == data_field::address)
        result = std::from_chars(line.data() + 3, line.data() + 7, out, 16);
    else if constexpr (F == data_field::record_type)
        result = std::from_chars(line.data() + 7, line.data() + 9, out, 16);
    else if constexpr (F == data_field::extended_address)
        result = std::from_chars(line.data() + 9, line.data() + 13, out, 16);
    else {
        // Just die already :'-)
        assert(!"Invalid data field type");
        throw std::runtime_error("Invalid data field type");
    }

    if (result.ec == std::errc{})
        return out;
    return unexpected_error{std::make_error_code(result.ec).message()};
}

void_or_error verify_line(std::string_view line)
{
    if (line.size() < minimum_line_size)
        return unexpected_error{"minimum line length"};
    if (line.at(0) != ':')
        return unexpected_error{"illegal start code '"s + line.at(0) + "'"};
    line.remove_prefix(1);

    // A record's checksum byte is the two's complement of the least significant
    // byte of the sum of all decoded byte values in the record preceding the checksum.
    // Since the record's checksum byte is the two's complement — and therefore the
    // additive inverse — of the data checksum, this process can be reduced to summing
    // all decoded byte values, including the record's checksum, and verifying that
    // the LSB of the sum is zero.
    //
    // For example, the record :042ffc00ffffff7f55 the checksum can be verified as follows:
    // - Sum decoded bytes and take LSB:    LSB(04 + 2f + fc + 00 + ff + ff + ff + 7f) = ab
    // - Take two's complement of LSB:      twocomplement(ab) = ~ab + 1 = 54 + 1 = 55
    // - Or, via the simplified method:     LSB(04 + 2f + fc + 00 + ff + ff + ff + 7f + 55) = 00

    unsigned char checksum = 0;
    unsigned char byte;
    for (auto it = line.begin(); it != line.end(); it += 2) {
        auto result = std::from_chars(it, it + 2, byte, 16);
        if (result.ec != std::errc{})
            return unexpected_error{std::make_error_code(result.ec).message()};
        checksum = static_cast<unsigned char>(checksum + byte);
    }

    if (checksum)
        return unexpected_error{"checksum incorrect"};
    return {};
}

unsigned char * safe_zero_alloc(std::size_t size)
{
    // It looks like calloc is quicker than new + memset
    void * mem = std::calloc(size, 1);
    if (!mem)
        throw std::runtime_error("Unable to allocate memory");
    return reinterpret_cast<unsigned char *>(mem);
}

} // End of namespace

namespace hal::rpi_cube
{

void_or_error verify(memory_layout const & layout)
{
    if (layout.start_address >= layout.end_address)
        return unexpected_error{"start address is greater than or equal to end address"};
    auto const size = layout.end_address - layout.start_address;
    if (size > (1024 * 1024 * 4))
        return unexpected_error{"size cannot exceed 4MiB"};
    if (layout.word_size) {
        if (*layout.word_size != 1 && *layout.word_size != 2 && *layout.word_size != 4)
            return unexpected_error{"word size must be either 1, 2 or 4"};
        if (size % *layout.word_size != 0)
            return unexpected_error{"size not divisible by word size"};
    }
    if (layout.row_size) {
        if (*layout.row_size < 4)
            return unexpected_error{"row size cannot be smaller than 4"};
        if (size % *layout.row_size != 0)
            return unexpected_error{"size not divisible by row size"};
    }
    if (layout.page_size) {
        if (*layout.page_size < 4)
            return unexpected_error{"page size cannot be smaller than 4"};
        if (layout.row_size && *layout.page_size < *layout.row_size)
            return unexpected_error{"page size cannot be smaller than row size"};
        if (size % *layout.page_size != 0)
            return unexpected_error{"size not divisible by page size"};
    }
    return {};
}

expected_or_error<memory_blob> parse_hex_file(fs::path const & filepath, memory_layout layout)
{
    auto ok = verify(layout);
    if (!ok)
        return unexpected_error{"Invalid memory layout: " + ok.error().what};
    if (!fs::exists(filepath))
        return unexpected_error{"File does not exist: " + filepath.string()};
    std::ifstream file(filepath);
    if (!file.is_open())
        return unexpected_error{"Failed to open file: " + filepath.string()};

    uint32_t const blob_size = layout.end_address - layout.start_address;
    byte_array_t byte_array{safe_zero_alloc(blob_size)};

    std::string line;
    std::size_t line_nr = 0;
    uint32_t address_h = 0;
    bool eof = false;

    auto make_error = [&](std::string const & what) -> unexpected_error
    {
        return {"Error at line '"s + std::to_string(line_nr) + "': " + what};
    };

    while (std::getline(file, line) && !eof) {
        line_nr++;

        ok = verify_line(line);
        if (!ok)
            return make_error("invalid line: " + ok.error().what);
        auto byte_count = parse<data_field::byte_count>(line);
        if (!byte_count)
            return make_error("invalid byte count field: " + byte_count.error().what);
        if (line.size() != (*byte_count * 2 + minimum_line_size))
            return make_error("invalid line length for byte count");
        auto address_l = parse<data_field::address>(line);
        if (!address_l)
            return make_error("invalid address field: " + address_l.error().what);
        auto type = parse<data_field::record_type>(line);
        if (!type)
            return make_error("invalid record type field: " + type.error().what);

        switch (static_cast<record_type>(*type)) {
            default:
                return make_error("unsupported record type: " + std::to_string(*type));
            case record_type::eof:
                eof = true;
                break;
            case record_type::data: {
                uint32_t address = (address_h & 0xffff0000) | (*address_l & 0x0000ffff);
                if (address >= layout.start_address && (address + *byte_count) <= layout.end_address) {
                    std::size_t index = (address - layout.start_address);
                    std::string_view data = data_view(line);
                    for (auto it = data.begin(); it != data.end(); it += 2) {
                        auto result = std::from_chars(it, it + 2, byte_array.get()[index++], 16);
                        if (result.ec != std::errc{})
                            return unexpected_error{std::make_error_code(result.ec).message()};
                    }
                }
                break;
            }
            case record_type::extended_linear_address: {
                auto address = parse<data_field::extended_address>(line);
                if (!address)
                    return make_error("invalid extended address field: " + address.error().what);
                address_h = *address << 16;
                break;
            }
        }
    }

    return memory_blob{blob_size, std::move(byte_array)};
}

} // End of namespace
