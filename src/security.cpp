#include "security.hpp"
#include <iomanip>
#include "wallet.hpp"
#include "cli.hpp"
#include "Keccak256.hpp"
#include "json.hpp"
#define ITERATIONS 262144
using json = nlohmann::json;
namespace security_manager {
bool first_time_save(const Wallet& wallet, const std::string& filename) {
bytes_data password = cli::read_and_confirm_password();

bytes_data salt = crypto_utils::genNumber(16);
bytes_data hash_key = crypto_utils::PBKDF2_HMAC_SHA512(password, salt, ITERATIONS);
bytes_data encryption_key, mac_key;
crypto_utils::split_key(hash_key, encryption_key, mac_key);

const bytes_data& masternode = wallet.get_master_node();
bytes_data iv = crypto_utils::genNumber(16);
bytes_data encrypted_masternode = crypto_utils::AES_256_CTR(encryption_key, masternode,iv);
bytes_data mac_input, mac(32);
mac_input.insert(mac_input.end(), mac_key.begin() + 16, mac_key.end());
mac_input.insert(mac_input.end(), encrypted_masternode.begin(), encrypted_masternode.end());
Keccak256::getHash(mac_input.data(), mac_input.size(), mac.data());

EncryptedKeystore encrp(encrypted_masternode, mac, iv, salt, ITERATIONS);
encrp.save(filename);

OPENSSL_cleanse(password.data(), password.size());
OPENSSL_cleanse(hash_key.data(), hash_key.size());
OPENSSL_cleanse(encryption_key.data(), encryption_key.size());
OPENSSL_cleanse(mac_key.data(), mac_key.size());

return false;
}

bool load_wallet(Wallet& wallet,const  std::string&filename) {
  std::ifstream file(filename);

  if(!file.is_open()) {
    return false;
  }
  try {
  json j;
  file >> j;

  wallet.set_eth_address(j["current_address"]);
  wallet.set_master_node(j["master_node"]);
  } catch(const std::exception& err) {
    std::cerr << err.what() << std::endl;

  }
  return true;
}
} // namespace security_manager