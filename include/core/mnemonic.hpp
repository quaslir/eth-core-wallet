#include "config/config.hpp"
#include "utils/crypto_utils.hpp"
#include "utils/tech_utils.hpp"
#include <string_view>
#include "secure_bytes_data.hpp"

class MnemonicGenerator {
private:
  crypto_utils::Hashes hashes;

  static bytes_data createMnemonic(const bytes_data &randNumber,
                                   const bytes_data &checkSum);
  static bytes_data createSalt(const bytes_data &passphrase);

public:
  MnemonicGenerator();
  bytes_data generateSeed(const bytes_data &mnemonic,
                          const bytes_data &passphrase = {}) const;
  bytes_data generateMnemonic(const Config &conf) const;
  bool mnemonic_is_correct(std::string_view mnemonic) const;
  bytes_data handle_extra_entropy_from_user(const bytes_data &entropy,
                                            const bytes_data &extra_entropy,
                                            int target_bits) const;

  // Folowing function was created only for tests, it isn't used in the main
  // application.
  bytes_data __generateMnemonic(const bytes_data &entropy) const;
};
