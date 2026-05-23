#include "utils/tech_utils.hpp"
#include "core/secure_bytes_data.hpp"
#include "drivers/balance_client.hpp"

#include <cctype>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <fmt/core.h>
#include <openssl/crypto.h>
#include <string>
#include <system_error>
namespace tech_utils {

secure_string to_hex(const bytes_data &data) {
  secure_string hex_format;
  hex_format.reserve(data.size());

  for (const auto &byte : data) {
    hex_format.append(fmt::format("{:02x}", byte));
  }

  return hex_format;
}

bytes_data to_hex_bytes(const bytes_data &data) {
  static const char hex_chars[] = "0123456789abcdef";
  bytes_data hex_format;
  hex_format.reserve(data.size() * 2);

  for (const auto &byte : data) {
    hex_format.push_back(static_cast<uint8_t>(hex_chars[(byte >> 4) & 0x0F]));
    hex_format.push_back(static_cast<uint8_t>(hex_chars[byte & 0x0F]));
  }

  return hex_format;
}

bytes_data from_hex_to_bytes(std::string hex) {
  if (hex.starts_with("0x") || hex.starts_with("0X"))
    hex = hex.substr(2);
  if (hex.length() % 2 != 0) {
    throw std::runtime_error("Invalid hex string");
  }

  bytes_data bytes;
  bytes.reserve(hex.length() / 2);

  for (size_t i = 0; i < hex.length(); i += 2) {
    std::string bytesStr = hex.substr(i, 2);

    uint8_t byte = static_cast<uint8_t>(std::stoul(bytesStr, nullptr, 16));

    bytes.push_back(byte);
  }

  return bytes;
}

bytes_data to_bits(const std::vector<uint16_t> &data) {
  bytes_data binaryData;
  binaryData.reserve(data.size() * 11);

  for (const auto &byte2 : data) {
    for (int i = 10; i >= 0; i--) {
      binaryData.push_back(byte2 >> i & 1);
    }
  }
  return binaryData;
}

bytes_data to_bytes_from_bits(const bytes_data &data) {
  if (data.size() % 8 != 0) {
    return {};
  }

  bytes_data new_data;
  new_data.reserve(data.size() / 8);
  for (size_t i = 0; i < data.size(); i += 8) {
    uint8_t byte = 0;
    for (size_t j = 0; j < 8; j++) {
      byte = (byte << 1) | (data[j + i] & 1);
    }
    new_data.push_back(byte);
  }

  return new_data;
}

uint32_t parse_uint32(const std::string &data) {
  uint32_t result = 0;

  std::from_chars(data.data(), data.data() + data.size(), result);
  return result;
}

void trim(std::string &data) {
  size_t end = data.find_last_not_of(" \t\r\n");

  if (end == std::string::npos) {
    data.clear();
    return;
  }
  data.erase(end + 1);
  size_t start = data.find_first_not_of(" \t\r\n");

  data.erase(0, start);
}
void rm_file(const std::string &filename) { std::filesystem::remove(filename); }

void clear(bytes_data &data) { OPENSSL_cleanse(data.data(), data.size()); }

bool to_double(const std::string &str, double &val) {
  try {
    std::size_t pos;
    double result = std::stod(str, &pos);

    if (pos != str.size())
      return false;

    val = result;

    return true;
  } catch (...) {
    return false;
  }
}

double eth_to_usd(double eth, double price) { return eth * price; }

bool contains_only_lowercase(std::string_view string) {
  if (string.empty() || string.front() == ' ' || string.back() == ' ')
    return false;

  for (size_t i = 0; i < string.size(); i++) {
    char c = string[i];
    if (c == ' ') {

      if (string[i + 1] == ' ')
        return false;

    } else if (c < 'a' || c > 'z')
      return false;
  }

  return true;
}

std::string decimals_to_divisor(int decimals) {
  if (decimals < 0)
    return "1";

  return "1" + std::string(decimals, '0');
}

double calculate_total(const assets_data &assets) {
  double total = 0.0;
  for (const auto &asset : assets) {
    total += (asset.second.fiat_price * asset.second.balance);
  }

  return total;
}

uint64_t string_to_uint64(const std::string &str) {
  uint64_t value = 0;

  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);

  if (ec == std::errc() && ptr == str.data() + str.size()) {
    return value;
  }

  return 0;
}

} // namespace tech_utils
