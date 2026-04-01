#include "mnemonic.hpp"
#include "utils.hpp"
#include "derive.hpp"
#include <iomanip>
#include <iostream>
#include <vector>
int main(void) {
  MnemonicGenerator mem;
  std::vector<uint8_t> seed = mem.generateSeed(128);
  std::string_view key = "Bitcoin seed";
  std::vector<uint8_t> masterNode = crypto_utils::HMAC_SHA512(key, seed);
  OPENSSL_cleanse(seed.data(), seed.size());

  std::vector<uint8_t> private_key, chain_key;
  private_key.resize(32);
  chain_key.resize(32);

  crypto_utils::split_key(masterNode, private_key, chain_key);
  Key_Derive devk;
  for(size_t i = 0; i < 5; i++) {
    devk.derive_child(private_key, chain_key, crypto_utils::path_deriv[i]);
  }
  return 0;
}