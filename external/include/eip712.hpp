#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace eip712 {

using bytes_t = std::vector<uint8_t>;

bytes_t keccak256(const bytes_t& data);

std::string encode_type(const std::string& primary_type, const nlohmann::json& types);
bytes_t type_hash(const std::string& primary_type, const nlohmann::json& types);
bytes_t encode_value(const std::string& type, const nlohmann::json& value, const nlohmann::json& types);
bytes_t hash_struct(const std::string& primary_type, const nlohmann::json& data, const nlohmann::json& types);
bytes_t hash_typed_data(const nlohmann::json& typed_data);

} // namespace eip712
