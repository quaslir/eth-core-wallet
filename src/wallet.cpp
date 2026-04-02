#include "wallet.hpp"

void Wallet::generate(int strength) {
    (void) strength;
     MnemonicGenerator mem;
  bytes_data seed = mem.generateSeed(128);
  std::string_view key = "Bitcoin seed";
  bytes_data masterNode = crypto_utils::HMAC_SHA512(key, seed);
  OPENSSL_cleanse(seed.data(), seed.size());

  bytes_data private_key(32), chain_key(32);

  KEY_PAIR keys;
  crypto_utils::split_key(masterNode, keys.parent_key, keys.chain_key);
  Key_Derive devk;

  for (size_t i = 0; i < 5; i++) {
    devk.derive_child(keys, crypto_utils::path_deriv[i]);
  }

eth_address = devk.generate_address(keys.parent_key);
priv_key = std::move(keys.parent_key);
}

 bytes_data Wallet::get_eth_address(void) const {
    return this->eth_address;
 }
    bytes_data Wallet::get_private_key(void) const {
        return this->priv_key;
    }