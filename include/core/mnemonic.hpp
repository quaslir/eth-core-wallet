#include "config/config.hpp"
#include "utils/crypto_utils.hpp"
#include "utils/tech_utils.hpp"
#include <string_view>
#include <vector>
using bytes_data = std::vector<uint8_t>;
class MnemonicGenerator {
private:
  crypto_utils::Hashes hashes;

  static bytes_data createMnemonic(bytes_data &randNumber,
                                   bytes_data &checkSum);
  static bytes_data createSalt(bytes_data &passphrase);

public:
  MnemonicGenerator();
  bytes_data generateSeed(bytes_data &mnemonic, bytes_data &passphrase) const;
  bytes_data generateMnemonic(Config &conf) const;
  bool mnemonic_is_correct(std::string_view mnemonic) const;
  bytes_data handle_extra_entropy_from_user(bytes_data &entropy,
                                            bytes_data &extra_entropy,
                                            int target_bits) const;
};
