#include "config/config.hpp"
#include "core/secure_bytes_data.hpp"


void Config::set_bit_length(int bt) { bit_length = bt; }

void Config::set_extra_entropy(bytes_data && entropy) {
  extra_entropy = std::move(entropy);
}
void Config::set_passphrase(secure_string &&pass) { passphrase = std::move(pass); }
void Config::change_derivation_path(secure_string&& derive_path) {
  derivation_path = std::move(derive_path);
}
