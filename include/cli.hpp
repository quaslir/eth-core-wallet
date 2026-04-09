#include "config.hpp"
#include "wallet.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

using bytes_data = std::vector<uint8_t>;
using namespace ftxui;
namespace cli {
bytes_data request_input_mnemonic(void);
bytes_data request_input_optional_passphrase(void);
Component createMainMenu(int *selected, std::string_view error_msg = "");

void display_mnemonic(std::string_view mnemonic);
Element render_mnemonic_element(std::string_view mnemonic);
bool confirm_liability_waiver(void);
Component render_mnemonic_wiping(std::shared_ptr<std::string> user_input,
                                 std::string_view error_msg);
void incorrect_mnemonic_text(void);
Component render_config_menu(const Config &cfg, int *selected);
void print_wallet_ui(const Wallet &wallet, std::string_view error_msg = "");
int handle_wallet_ui_input(const Wallet &wallet);
bool confirm_danger_action(void);
const bytes_data read_and_confirm_password(void);
const bytes_data request_unlock_password(size_t attempts, size_t max_attempts);
void show_self_destruct(void);
void display_private_key(const bytes_data &priv_key);
} // namespace cli