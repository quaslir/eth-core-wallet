#include <openssl/rand.h>
#include <stdint.h>
#include "utils.hpp"
#include <vector>
#include <string_view>
class MnemonicGenerator {
private:
  crypto_utils::Hashes hashes;

  std::vector<uint8_t> genNumber(size_t bytes) const;
  std::vector<uint8_t> createKey(std::vector<uint8_t> &&randNumber, std::vector<bool> &checkSum);

public:
  MnemonicGenerator();
  void generateSeedPhrase(uint16_t bits);
};