#include <fstream>
#include <nlohmann/json.hpp>
#include <stdint.h>
#include <string>
#include <vector>
using bytes_data = std::vector<uint8_t>;
using json = nlohmann::json;
struct EncryptedKeystore {
  bytes_data ciphertext;
  bytes_data mac;
  bytes_data iv;
  bytes_data salt;
  uint32_t iter;
  int index;
  json to_json(void) const;
  bool save(const std::string &filename);
  bool load(const std::string &filename);
};