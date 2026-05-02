#include "ui/cli.hpp"

#include "iwallet_actions.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>

void CLI::load(void) {
  auto main_menu = create_main_menu();
  auto mnemonic_display = render_mnemonic_element();
  auto mnenonic_wiping_confirmation = render_mnemonic_wiping();
  auto import_wallet_ui = render_import_mnemonic_component();
  auto optional_passphrase = render_input_optional_passphrase_component();
  auto config_menu = render_config_menu();
  auto set_password_component = render_password_setup();
  auto confirm_password_component = render_confirm_password_setup();
  auto password_unlock = render_request_unlock_password();
  auto wallet_ui = print_wallet_ui();
  auto bit_length_selection_config = set_bit_length();
  auto extra_entropy_selection_config = handle_extra_entropy();
  auto passphrase_selection_config = handle_passphrase();
  auto derive_path_selection_config = handle_derivation_path();
  auto display_priv_key = display_private_key();
  auto trans_history = transaction_history_render();
  auto network_changer = change_network_render();
  auto root_container = Container::Tab(
      {main_menu, config_menu, import_wallet_ui, optional_passphrase,
       mnemonic_display, mnenonic_wiping_confirmation, set_password_component,
       confirm_password_component, wallet_ui, password_unlock,
       bit_length_selection_config, extra_entropy_selection_config,
       passphrase_selection_config, derive_path_selection_config,
       display_priv_key, trans_history, network_changer},
      &this->active_tab);

  std::atomic<bool> refresh_ui = true;
  std::thread refresh_thread([&] {
    while (refresh_ui) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      actions->update_balance();
      actions->update_transactions_data();
      actions->update_eth_price();
      screen.PostEvent(Event::Custom);
    }
  });
  Screen::Cursor cursor;
  cursor.shape = Screen::Cursor::Shape::Hidden;
  screen.SetCursor(cursor);
  screen.Loop(root_container);

  refresh_ui = false;
  if (refresh_thread.joinable()) {
    refresh_thread.join();
  }
}
