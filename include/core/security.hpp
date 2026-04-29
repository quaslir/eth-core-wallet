#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
class Wallet;
using bytes_data = std::vector<uint8_t>;
namespace security_manager {
bool first_time_save(const Wallet &wallet, bytes_data &password,
                     const std::string &filename = "session.json");
bool update(const Wallet &wallet, const std::string &filename = "session.json");
bool load_wallet(Wallet &wallet, bytes_data &password,
                 const std::string &filename = "session.json");
} // namespace security_manager
