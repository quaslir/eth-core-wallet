#pragma once
#include "core/asset.hpp"
#include "core/secure_bytes_data.hpp"
#include "drivers/balance_client.hpp"
#include <cstdint>
#include <stdint.h>
#include <string>
#include <string_view>
#include <vector>
namespace tech_utils {
secure_string to_hex(const bytes_data &data);
bytes_data to_hex_bytes(const bytes_data &data);
bytes_data from_hex_to_bytes(std::string hex);
bytes_data to_bits(const std::vector<uint16_t> &data);
bytes_data to_bytes_from_bits(const bytes_data &data);
uint32_t parse_uint32(const std::string &data);
void trim(std::string &data);
void rm_file(const std::string &filename = "session.json");
void clear(bytes_data &data);
bool to_double(const std::string &str, double &val);
double eth_to_usd(double eth, double price);
bool contains_only_lowercase(std::string_view string);
std::string decimals_to_divisor(int decimals);
double calculate_total(const assets_data &assets);
uint64_t string_to_uint64(const std::string &str);
} // namespace tech_utils
