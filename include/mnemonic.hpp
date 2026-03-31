#include "utils.hpp"
#include <openssl/rand.h>
#include <stdint.h>
#include <string_view>
#include <vector>
class MnemonicGenerator {
private:
  crypto_utils::Hashes hashes;

  static std::vector<uint8_t> genNumber(size_t bytes);
  static std::vector<uint8_t> createMnemonic(std::vector<uint8_t> &&randNumber,
                                             std::vector<bool> &&checkSum);
  static std::vector<uint8_t> createSalt(std::string_view passphrase = "");

public:
  MnemonicGenerator();
  std::vector<uint8_t> generateSeed(uint16_t bits);
};