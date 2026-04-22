#include "json.hpp"
#include "tech_utils.hpp"
#include <fstream>
json EncryptedKeystore::to_json(void) const {
  json j;
  j["ciphertext"] = tech_utils::to_hex(ciphertext);
  j["mac"] = tech_utils::to_hex(mac);
  j["iv"] = tech_utils::to_hex(iv);
  j["salt"] = tech_utils::to_hex(salt);
  j["iter"] = iter;
  j["index"] = index;
  return j;
}

bool EncryptedKeystore::save(const std::string &filename) {
  std::ofstream file(filename);
  if (!file.is_open()) {
    return false;
  }

  json j = to_json();
  file << j << std::endl;
  return true;
}

bool EncryptedKeystore::load(const std::string &filename) {
  std::ifstream file(filename);

  if (!file.is_open())
    return false;
  try {
    json j;

    file >> j;
    ciphertext = tech_utils::from_hex_to_bytes(j["ciphertext"]);
    mac = tech_utils::from_hex_to_bytes(j["mac"]);
    iv = tech_utils::from_hex_to_bytes(j["iv"]);
    salt = tech_utils::from_hex_to_bytes(j["salt"]);
    iter = j["iter"];
    index = j["index"];
  } catch (...) {
    return false;
  }

  return true;
}

GetBalanceMethod::GetBalanceMethod(const std::string &jsonrpc,
                                   const std::string &method,
                                   const std::vector<std::string> &params,
                                   int id) {
  j["jsonrpc"] = jsonrpc;
  j["method"] = method;
  j["params"] = params;
  j["id"] = id;
}

std::string GetBalanceMethod::to_string(void) const { return j.dump(); }

void GetBalanceMethod::parse(const std::string &data) { j = json::parse(data); }

std::string GetBalanceMethod::get_result(void) const { return j["result"]; }

namespace transactions_history {
json form_receives(const std::string &addr) {
  return {{"id", 1},
          {"jsonrpc", "2.0"},
          {"method", "alchemy_getAssetTransfers"},
          {"params",
           {{{"fromBlock", "0x0"},
             {"toBlock", "latest"},
             {"toAddress", addr},
             {"category", {"external", "erc20"}},
             {"withMetadata", true},
             {"excludeZeroValue", true}}}}};
}
json form_sends(const std::string &addr) {
  return {{"id", 1},
          {"jsonrpc", "2.0"},
          {"method", "alchemy_getAssetTransfers"},
          {"params",
           {{{"fromBlock", "0x0"},
             {"toBlock", "latest"},
             {"fromAddress", addr},
             {"category", {"external", "erc20"}},
             {"withMetadata", true},
             {"excludeZeroValue", true}}}}};
}
} // namespace transactions_history
