#include "json.hpp"
#include <stdexcept>
#include "utils.hpp"
json EncryptedKeystore::to_json(void) const {
    json j;
    j["ciphertext"] = tech_utils::to_hex(ciphertext);
    j["mac"] = tech_utils::to_hex(mac);
    j["iv"] = tech_utils::to_hex(iv);
    j["salt"] = tech_utils::to_hex(salt);
    j["iter"] = iter;

    return j;
}

bool EncryptedKeystore::save(const std::string& filename) {
std::ofstream file(filename);
if(!file.is_open()) {
    return false;
}

json j = to_json();
file << j << std::endl;
return true;
}