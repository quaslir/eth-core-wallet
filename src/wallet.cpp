#include "wallet.hpp"
#include <iostream>
void Wallet::generate(int strength) {
    MnemonicGenerator mem;
  bytes_data mnemonic = mem.generateMnemonic(strength);
  

  bytes_data seed = mem.generateSeed(mnemonic);
  std::string_view key = "Bitcoin seed";
  bytes_data masterNode = crypto_utils::HMAC_SHA512(key, seed);
  OPENSSL_cleanse(seed.data(), seed.size());

  bytes_data private_key(32), chain_key(32);

  KEY_PAIR keys;
  crypto_utils::split_key(masterNode, keys.parent_key, keys.chain_key);
  master_node = masterNode;
  OPENSSL_cleanse(masterNode.data(), masterNode.size());
  Key_Derive devk;

  for (size_t i = 0; i < 5; i++) {
    devk.derive_child(keys, crypto_utils::path_deriv[i]);
  }

eth_address = devk.generate_address(keys.parent_key);

priv_key = keys.parent_key;
OPENSSL_cleanse(keys.parent_key.data(), keys.parent_key.size());

}

 bytes_data Wallet::get_eth_address(void) const {
    return this->eth_address;
 }
bytes_data Wallet::get_private_key(void) const {
        return this->priv_key;
    }
long long Wallet::getIndex(void) const {
return this->index;
}