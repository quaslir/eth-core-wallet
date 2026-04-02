#include "derive.hpp"
#include "mnemonic.hpp"
#include "utils.hpp"
#include <iomanip>
#include <iostream>
#include <vector>
int main(void) {
  MnemonicGenerator mem;
  std::vector<uint8_t> seed = mem.generateSeed(128);
  std::string_view key = "Bitcoin seed";
  std::vector<uint8_t> masterNode = crypto_utils::HMAC_SHA512(key, seed);
  OPENSSL_cleanse(seed.data(), seed.size());

  std::vector<uint8_t> private_key(32), chain_key(32);

  KEY_PAIR keys;
  crypto_utils::split_key(masterNode, keys.parent_key, keys.chain_key);
  Key_Derive devk;

  for (size_t i = 0; i < 5; i++) {
    devk.derive_child(keys, crypto_utils::path_deriv[i]);
  }

  std::cout << "PRIVATE_KEY: 0x";
  for (size_t i = 0; i < keys.parent_key.size(); i++) {
    printf("%02x", keys.parent_key[i]);
  }

  std::cout << "\nPUBLIC_KEY: 0x";
  std::vector<uint8_t> eth_addr = devk.generate_address(keys.parent_key);

  for (size_t i = 0; i < eth_addr.size(); i++) {
    printf("%02x", eth_addr[i]);
  }
  return 0;
}