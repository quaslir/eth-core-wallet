#pragma once
#include <stdint.h>
#include <string>
#include <string_view>
#include <vector>
using bytes_data = std::vector<uint8_t>;
struct Config {
public:
  bytes_data extra_entropy;
  int bit_length = 256;
  bytes_data passphrase;
  std::string derivation_path = "m/44'/60'/0'/0/0";

  void set_bit_length(int bt);
  void set_extra_entropy(std::string_view entropy);
  void set_passphrase(const bytes_data &pass = {});
  void change_derivation_path(std::string_view derive_path);
};
