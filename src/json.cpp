#include "json.hpp"
#include "utils.hpp"
#include <stdexcept>
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