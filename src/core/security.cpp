#include "core/security.hpp"
#include "Keccak256.hpp"
#include "api/json.hpp"
#include "core/wallet.hpp"
#include "ui/cli.hpp"
#include <string>
#define ITERATIONS 262144
using json = nlohmann::json;
namespace security_manager {
bool first_time_save(const Wallet &wallet, const bytes_data &password,
                     const std::string &filename) {

  bytes_data salt = crypto_utils::gen_number(16);
  bytes_data hash_key =
      crypto_utils::PBKDF2_HMAC_SHA512(password, salt, ITERATIONS);

  bytes_data encryption_key, mac_key;
  crypto_utils::split_key_64(hash_key, encryption_key, mac_key);

  const bytes_data &masternode = wallet.get_master_node();
  bytes_data iv = crypto_utils::gen_number(16);
  bytes_data encrypted_masternode =
      crypto_utils::AES_256_CTR(encryption_key, masternode, iv);
  bytes_data mac_input, mac(32);
  mac_input.insert(mac_input.end(), mac_key.begin() + 16, mac_key.end());
  mac_input.insert(mac_input.end(), encrypted_masternode.begin(),
                   encrypted_masternode.end());
  Keccak256::getHash(mac_input.data(), mac_input.size(), mac.data());

  EncryptedKeystore encrp(encrypted_masternode, mac, iv, salt, ITERATIONS,
                          wallet.getIndex());

  if (!encrp.save(filename))
    return false;

  return true;
}

bool update(const Wallet &wallet, const std::string &filename) {
  EncryptedKeystore encrp;
  if (!encrp.load(filename))
    return false;

  encrp.index = wallet.getIndex();
  if (!encrp.save(filename))
    return false;

  return true;
}

bool load_wallet(Wallet &wallet, const bytes_data &password,
                 const std::string &filename) {
  EncryptedKeystore encrp;
  if (!encrp.load(filename))
    return false;
  bool auth_failed = false;

  bytes_data hash_key =
      crypto_utils::PBKDF2_HMAC_SHA512(password, encrp.salt, encrp.iter);

  bytes_data mac_key, encryption_key, mac(32), mac_input;
  crypto_utils::split_key_64(hash_key, encryption_key, mac_key);

  mac_input.insert(mac_input.end(), mac_key.begin() + 16, mac_key.end());
  mac_input.insert(mac_input.end(), encrp.ciphertext.begin(),
                   encrp.ciphertext.end());
  Keccak256::getHash(mac_input.data(), mac_input.size(), mac.data());

  auth_failed = mac != encrp.mac;

  if (auth_failed) {
    return false;
  }

  bytes_data decrypted_masternode =
      crypto_utils::AES_256_CTR(encryption_key, encrp.ciphertext, encrp.iv);
  wallet.set_master_node(decrypted_masternode);
  wallet.set_index(encrp.index);
  return true;
}
} // namespace security_manager
