#include "config.hpp"
#include "wallet.hpp"
#include <iostream>
#include <string>
#include <vector>
using bytes_data = std::vector<uint8_t>;
namespace cli {
std::string request_input_mnemonic(void);
std::string request_input_optional_passphrase(void);
void print_welcome_message(void);
int make_choice_from_welcome_message(void);
void display_mnemonic(const bytes_data &mnemonic);
void confirm_liability_waiver(void);
void incorrect_mnemonic_text(void);
char render_config_menu(const Config &cfg);
void print_wallet_ui(const Wallet &wallet);
char handle_wallet_ui_input(void);
} // namespace cli