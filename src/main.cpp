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
  for (size_t i = 0; i < masterNode.size(); i++) {
    printf("%02x", masterNode[i]);
  }
  return 0;
}