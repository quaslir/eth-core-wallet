#include <nlohmann/json.hpp>
#include <vector>
#include <stdint.h>
#include <string>
#include <fstream>
using bytes_data = std::vector<uint8_t>;
using json = nlohmann::json;
struct EncryptedKeystore {
bytes_data ciphertext;
bytes_data mac;
bytes_data iv;
bytes_data salt;
uint32_t iter;

json to_json(void) const;
bool save(const std::string& filename);
};