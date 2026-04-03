#include <string>
#include <iostream>
#include <vector>
#include "config.hpp"
#include "wallet.hpp"
using bytes_data = std::vector<uint8_t>;
namespace cli {
    std::string request_input_mnemonic(void);
    std::string request_input_optional_passphrase(void);
    void print_welcome_message(void);
    void render_bit_length_menu(void);
    void display_mnemonic(const bytes_data& mnemonic);
    void confirm_liability_waiver(void);
    void incorrect_mnemonic_text(void);
    bytes_data receive_passphrase(void);
    char render_config_menu(const Config& cfg);
    void cli::print_wallet_ui(const Wallet& wallet);
}