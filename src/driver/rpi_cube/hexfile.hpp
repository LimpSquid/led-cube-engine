#pragma once

#include <cube/core/expected.hpp>
#include <filesystem>
#include <vector>

namespace driver::rpi_cube
{

struct byte_array_deleter
{
    // Because we're calloc'ing memory
    void operator()(unsigned char * p) { std::free(p); }
};
using byte_array_t = std::unique_ptr<unsigned char, byte_array_deleter>;

class memory_blob
{
public:
    memory_blob(uint32_t size, byte_array_t && data) :
        size_(size),
        data_(std::move(data))
    { }

    unsigned char * begin() { return data_.get(); }
    unsigned char const * begin() const { return data_.get(); }
    unsigned char * end() { return begin() + size_; }
    unsigned char const * end() const { return begin() + size_; }
    uint32_t size() const { return size_; }

private:
    uint32_t size_;
    byte_array_t data_;
};

struct memory_layout
{
    memory_layout() :
        start_address(0),
        end_address(1024 * 1024 * 4)
    { }

    bool operator==(memory_layout const & other) const
    {
        return start_address == other.start_address
            && end_address == other.start_address
            && word_size == other.word_size
            && row_size == other.row_size
            && page_size == other.page_size;
    }

    uint32_t start_address; // Inclusive
    uint32_t end_address; // Exclusive
    std::optional<uint32_t> word_size;
    std::optional<uint32_t> row_size;
    std::optional<uint32_t> page_size;
};

std::string to_string(memory_layout const & layout);
cube::core::void_or_error verify(memory_layout const & layout);
cube::core::expected_or_error<memory_blob> parse_hex_file(std::filesystem::path const & filepath, memory_layout layout = {});

} // End of namespace
