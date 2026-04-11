#include "config.hpp"
#include "wallet.hpp"
#include <cstddef>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
using bytes_data = std::vector<uint8_t>;
using namespace ftxui;


enum state_t {
  MAIN_MENU = 0,
  CONFIG_MENU = 1,
  IMPORT_MENU = 2,
  MNEMONIC_DISPLAY = 3,
  MNEMONIC_WIPING = 4,
  SET_PASSWORD = 5,
  CONFIRM_PASSWORD = 6,
  WALLET_UI = 7,
  UNLOCK_PASSWORD = 8
};



class CLI {
private:
  ScreenInteractive screen = ScreenInteractive::Fullscreen();
  int active_tab = MAIN_MENU;
  std::function<size_t(void)> get_attempts;
  std::function<size_t(void)> get_max_attempts;

  Component render_password_setup(void);
  Component create_main_menu(void);
  Component
  render_confirm_password_setup(void);
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
  std::function<void(bytes_data& pass)> set_password_for_wallet;
  std::function<bytes_data(void)> get_password_for_wallet;
  void load(void);
  bytes_data request_input_mnemonic(void);
  bytes_data request_input_optional_passphrase(void);
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