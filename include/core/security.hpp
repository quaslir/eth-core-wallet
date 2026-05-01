#pragma once
#include "core/secure_bytes_data.hpp"
#include <nlohmann/json.hpp>
#include <string>
class Wallet;

namespace security_manager {
bool first_time_save(const Wallet &wallet, const secure_string &password,
                     const std::string &filename = "session.json");
bool update(const Wallet &wallet, const std::string &filename = "session.json");
bool load_wallet(Wallet &wallet, const secure_string &password,
                 const std::string &filename = "session.json");
} // namespace security_manager
