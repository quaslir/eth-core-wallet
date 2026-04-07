#include "config.hpp"
#include "wallet.hpp"
#include <iostream>
#include <string>
#include <vector>
using bytes_data = std::vector<uint8_t>;
namespace cli {
std::string request_input_mnemonic(void);
std::string request_input_optional_passphrase(void);
void print_welcome_message(std::string_view error_msg = "");
int make_choice_from_welcome_message(void);
void display_mnemonic(const bytes_data &mnemonic);
void confirm_liability_waiver(std::string_view error_msg = "");
void incorrect_mnemonic_text(void);
char render_config_menu(const Config &cfg);
void print_wallet_ui(const Wallet &wallet, std::string_view error_msg = "");
int handle_wallet_ui_input(const Wallet &wallet);
bool confirm_danger_action(void);
const bytes_data read_and_confirm_password(void);
const bytes_data request_unlock_password(size_t attempts, size_t max_attempts);
void show_self_destruct(void);
void display_private_key(const bytes_data& priv_key);
} // namespace cli