#include "config/config.hpp"
#include "secure_bytes_data.hpp"
#include "utils/crypto_utils.hpp"

class MnemonicGenerator {
private:
  crypto_utils::Hashes hashes;

  static secure_string createMnemonic(const bytes_data &randNumber,
                                      const bytes_data &checkSum);
  static bytes_data createSalt(const secure_string &passphrase);

public:
  MnemonicGenerator();
  bytes_data generateSeed(const secure_string &mnemonic,
                          const secure_string &passphrase = "") const;
  secure_string generateMnemonic(const Config &conf) const;
  bool mnemonic_is_correct(const secure_string &mnemonic) const;
  bytes_data handle_extra_entropy_from_user(const bytes_data &entropy,
                                            const bytes_data &extra_entropy,
                                            int target_bits) const;

  // Folowing function was created only for tests, it isn't used in the main
  // application.
  secure_string __generateMnemonic(const bytes_data &entropy) const;
};
