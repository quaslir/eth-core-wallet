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
  
 

   std::cout << "PRIVATE_KEY: 0x";
  for(size_t i = 0; i < private_key.size(); i++) {
    printf("%02x", private_key[i]);
  }

    std::cout << "\nPUBLIC_KEY: 0x";
    std::vector<uint8_t> eth_addr = devk.generate_address(private_key);

    for(size_t i = 0; i < eth_addr.size(); i++) {
    printf("%02x", eth_addr[i]);
  }
  return 0;
}