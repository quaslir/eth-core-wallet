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

  void handle_user_entropy(void);
  void handle_bit_length(void);
  void handle_use_passphrase(void);
  void handle_derivation_path(void);
  void handle_extra_entropy(void);
  void set_passphrase(const bytes_data& pass = {});
  void change_derivation_path(std::string_view derive_path);
private:
  void render_passphrase_menu(std::string_view error_msg) const;
  void render_passphrase_input_screen() const;
  void render_bit_length_menu(std::string_view error_msg) const;
  void render_extra_entropy_menu(std::string_view error_msg) const;
  void render_input_extra_entropy_menu(void) const;
  void render_derivation_menu(std::string_view error_msg) const;
  void render_custom_path_input(std::string_view error_msg) const;
};
