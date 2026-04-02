#include "utils.hpp"
#include <openssl/rand.h>
#include <stdint.h>
#include <string_view>
#include <vector>
using bytes_data = std::vector<uint8_t>;
class MnemonicGenerator {
private:
  crypto_utils::Hashes hashes;

  static bytes_data genNumber(size_t bytes);
  static bytes_data createMnemonic(bytes_data &randNumber,
                                             bytes_data &checkSum);
  static bytes_data createSalt(std::string_view passphrase = "");

public:
  MnemonicGenerator();
  bytes_data generateSeed(bytes_data & mnemonic);
  bytes_data generateMnemonic(uint16_t bits);
};