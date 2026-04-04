#include "utils.hpp"
#include <openssl/rand.h>
#include <stdint.h>
#include <string_view>
#include "config.hpp"
#include <vector>
using bytes_data = std::vector<uint8_t>;
class MnemonicGenerator {
private:
  crypto_utils::Hashes hashes;

  static bytes_data genNumber(size_t bytes);
  static bytes_data createMnemonic(bytes_data &randNumber,
                                   bytes_data &checkSum);
  static bytes_data createSalt(bytes_data &passphrase);

public:
  MnemonicGenerator();
  bytes_data generateSeed(bytes_data &mnemonic, bytes_data &passphrase);
  bytes_data generateMnemonic(Config& conf);
  bool mnemonic_is_correct(std::string_view mnemonic);
  bytes_data handle_extra_entropy_from_user(bytes_data& entropy, bytes_data& extra_entropy, int target_bits);
};