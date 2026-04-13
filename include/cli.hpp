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
#include "iwallet_actions.hpp"
using bytes_data = std::vector<uint8_t>;
using namespace ftxui;

enum state_t {
  MAIN_MENU = 0,
  CONFIG_MENU = 1,
  IMPORT_MENU = 2,
  ENTER_OPTIONAL_PASSPHRASE_MENU = 3,
  MNEMONIC_DISPLAY = 4,
  MNEMONIC_WIPING = 5,
  SET_PASSWORD = 6,
  CONFIRM_PASSWORD = 7,
  WALLET_UI = 8,
  UNLOCK_PASSWORD = 9,
  BIT_LENGTH_CONFIG = 10
};

class CLI {
private:
  ScreenInteractive screen = ScreenInteractive::Fullscreen();
  int active_tab = MAIN_MENU;
  IWalletActions * actions;

  Component render_password_setup(void);
  Component create_main_menu(void);
  Component render_confirm_password_setup(void);
  Component render_config_menu(void);
  Component render_request_unlock_password(void);
  Component print_wallet_ui(void);
  Component render_import_mnemonic_component(void);
  Component render_input_optional_passphrase_component(void);
  Component render_mnemonic_element(void);
  Element to_center(Element box);
  Component render_mnemonic_wiping(void);
  Component set_bit_length(void);

public:

void set_actions(IWalletActions * act);
  void load(void);
  void set_active_tab(int tab);
};
