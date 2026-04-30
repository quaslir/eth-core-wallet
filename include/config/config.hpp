#pragma once
#include <string>
#include <string_view>
#include <cstdint>
#include "core/secure_bytes_data.hpp"

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
