#include "config.hpp"
#include "crypto_utils.hpp"
#include "tech_utils.hpp"

#define ERROR_MSG "\033[1;31m[!] Invalid choice.\033[0m\n\n"
void Config::set_bit_length(int bt) { bit_length = bt; }

void Config::set_extra_entropy(std::string_view entropy) {
  extra_entropy = bytes_data(entropy.begin(), entropy.end());
}
void Config::set_passphrase(const bytes_data &pass) { passphrase = pass; }
void Config::change_derivation_path(std::string_view derive_path) {
  derivation_path = derive_path;
}
