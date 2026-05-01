#pragma once
#include <string>
#include <string_view>
#include <cstdint>
#include "core/secure_bytes_data.hpp"

struct Config {
public:
  bytes_data extra_entropy;
  int bit_length = 256;
  secure_string passphrase;
  secure_string derivation_path = "m/44'/60'/0'/0/0";

  void set_bit_length(int bt);
  void set_extra_entropy(bytes_data && entropy);
  void set_passphrase(secure_string && passphrase = "");
  void change_derivation_path(secure_string && derive_path);
};
