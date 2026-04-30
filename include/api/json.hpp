#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include "core/secure_bytes_data.hpp"
using json = nlohmann::json;
struct EncryptedKeystore {
  bytes_data ciphertext;
  bytes_data mac;
  bytes_data iv;
  bytes_data salt;
  uint32_t iter;
  int index;
  json to_json(void) const;
  bool save(const std::string &filename = "session.json");
  bool load(const std::string &filename = "session.json");
};

struct GetBalanceMethod {
  json j;

  GetBalanceMethod(const std::string &jsonrpc, const std::string &method,
                   const std::vector<std::string> &params, int id);
  std::string to_string(void) const;
  void parse(const std::string &data);
  std::string get_result(void) const;
};

namespace transactions_history {
json form_receives(const std::string &addr);
json form_sends(const std::string &addr);
} // namespace transactions_history
