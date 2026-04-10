#include "config.hpp"
#include "wallet.hpp"
#include <cstddef>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>
#include <string>
#include <string_view>
#include <vector>
using bytes_data = std::vector<uint8_t>;
using namespace ftxui;
class CLI {
private:
  ScreenInteractive screen = ScreenInteractive::Fullscreen();
  int active_tab = 0;
  std::function<size_t(void)> get_attempts;
  std::function<size_t(void)> get_max_attempts;

  Component render_password_setup(std::shared_ptr<std::string> user_input);
  Component create_main_menu(void);
  Component
  render_confirm_password_setup(std::shared_ptr<std::string> user_input,
                                bool incorrect);
  Component render_config_menu(void);
  Component render_request_unlock_password(void);
  Component print_wallet_ui(void);

public:
  std::function<std::string(void)> get_mnemonic;
  std::function<const Config &(void)> get_config;
  std::function<void(int)> handle_config_menu;
  std::function<const Wallet &(void)> get_wallet;
  std::function<void(int)> on_main_menu;
  std::function<void(void)> handle_wallet_creation;
  void load(void);
  bytes_data request_input_mnemonic(void);
  bytes_data request_input_optional_passphrase(void);
  Component createMainMenu(int *selected, std::string_view error_msg = "");
  int handle_main_menu(void);
void set_active_tab(int tab);
  Component render_mnemonic_element(void);
  bool confirm_liability_waiver(void);
  Component render_mnemonic_wiping(void);

  void incorrect_mnemonic_text(void);

  int handle_wallet_ui_input(const Wallet &wallet);
  bool confirm_danger_action(void);
  const bytes_data read_and_confirm_password(void);
  const bytes_data request_unlock_password(size_t attempts,
                                           size_t max_attempts);
  void show_self_destruct(void);
  void display_private_key(const bytes_data &priv_key);
};