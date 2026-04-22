#pragma once
#include <stdint.h>
#include <string>
#include <vector>

using bytes_data = std::vector<uint8_t>;
namespace tech_utils {
void clear_stdin(void);
std::string read_stdin(void);
void print_hex(const bytes_data &data);
std::string to_hex(const bytes_data &data);
bytes_data to_hex_bytes(const bytes_data &data);
bytes_data from_hex_to_bytes(const std::string &hex);
bytes_data to_bits(const std::vector<uint16_t> &data);
bytes_data to_bytes_from_bits(const bytes_data &data);
uint32_t parse_uint32(const std::string &data);
void trim(std::string &data);
void rm_file(const std::string &filename = "session.json");
} // namespace tech_utils
