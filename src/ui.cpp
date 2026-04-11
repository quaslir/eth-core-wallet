#include "ui.hpp"
#include "async_manager.hpp"

#include "config.hpp"
#include <string>
#include <string_view>
#include <utility>

void UserInterface::load(void) {
  //handle_wallet_loading();
  set_callbacks_for_cli();
  cli.load();
}

void UserInterface::handle_wallet_loading(void) {
  if (security_manager::load_wallet(wallet)) {
    std::vector<uint32_t> current_path =
        crypto_utils::change_derive_path(wallet.getIndex());
    wallet.sync_derive_path(current_path);
    wallet.derive(current_path);

  }
}


void UserInterface::handle_wallet_creation(void) {
  cli.set_active_tab(CONFIG_MENU);
  if (0) { // FIX

    return;
  }
}

void UserInterface::handle_wallet_import(void) {
  bytes_data mnemonic = {}; // TO FIX
  std::string_view mnemonic_view(
      reinterpret_cast<const char *>(mnemonic.data()), mnemonic.size());
  if (mnemonic.empty()) {

    return;
  }
  while (!wallet.correct_mnemonic(mnemonic_view)) {
    cli.incorrect_mnemonic_text();
    mnemonic = cli.request_input_mnemonic();

    if (mnemonic.empty()) {

      return;
    }
  }

  bytes_data passphrase = cli.request_input_optional_passphrase();
  wallet.import_wallet(mnemonic, passphrase); // clears mnemonic and passphrase

}

void UserInterface::apply_choice_from_wallet_ui(int choice) {
  switch (choice) {
  case 1:
    break; // send transaction
  case 2:
    wallet.derive_next();
    break;

  case 3:
    wallet.derive_prev();
    break;
  case 4: // show private_key
    if (cli.confirm_danger_action()) {
      cli.display_private_key(wallet.get_private_key());
    }
    break;
  case 5: // exit
    wallet.save();

    break;
  }
}

void UserInterface::set_callbacks_for_cli(void) {
cli.get_config = [&](void) -> const Config& {
  return this->config;
};
cli.handle_config_menu = [&](int choice)-> void  {
if (choice == 1) {
        config.handle_extra_entropy();
      } else if (choice == 2) {
        config.handle_bit_length();
      } else if (choice == 3) {
        config.handle_use_passphrase();
      } else if (choice == 4) {
        config.handle_derivation_path();
      } else if (choice == 5) {
        cli.set_active_tab(MAIN_MENU);
      }

      else if (choice == 6) {
        bytes_data mnemonic = wallet.prepare_mnemonic(config);
  std::string view_mnemonic(reinterpret_cast<const char *>(mnemonic.data()), mnemonic.size());
cli.get_mnemonic = [view_mnemonic](void) -> std::string {
return view_mnemonic;
};
cli.set_active_tab(MNEMONIC_DISPLAY);

  const std::vector<uint32_t> PATH_DERIVE =
      Key_Derive::parse_derive_path(config.derivation_path);
  wallet.finalize_from_mnemonic(mnemonic, config.passphrase, PATH_DERIVE);

      }
};

cli.get_wallet = [&](void) -> const Wallet& {
  return this->wallet;
};

cli.on_main_menu = [&](int choice) -> void {

switch (choice) {

  case 1:
    handle_wallet_creation();
    break;
  case 2:

    //handle_wallet_import();
    break;
  case 3:
    exit(0);
  }
};

cli.get_password_for_wallet = [&](void) ->bytes_data {
return this->temp.password_for_wallet_unlocking;
};

cli.set_password_for_wallet = [&](bytes_data &pass) {
  this->temp.password_for_wallet_unlocking = pass;
};
}
