#include "ui.hpp"
#include "cli.hpp"
#include "config.hpp"
#include <iostream>
void UserInterface::load(void) {
  handle_wallet_loading();
  while (state != EXIT) {

    if (state == MAIN_MENU) {
      int choice = cli::make_choice_from_welcome_message();
      apply_choice_from_welcome_message(choice);
    } else if (state == WALLET_UI) {
      int ch = cli::handle_wallet_ui_input(wallet);
      apply_choice_from_wallet_ui(ch);
    }
  }
}

void UserInterface::handle_wallet_loading(void) {
  if (security_manager::load_wallet(wallet)) {
    std::vector<uint32_t> current_path =
        crypto_utils::change_derive_path(wallet.getIndex());
    wallet.sync_derive_path(current_path);
    wallet.derive(current_path);
    state = WALLET_UI;
  }
}

void UserInterface::apply_choice_from_welcome_message(int choice) {
  switch (choice) {
  case 1:
    state = SEED_GENERATION;
    handle_wallet_creation();
    break;
  case 2:
    state = SEED_IMPORT;
    handle_wallet_import();
    break;
  case 3:
    exit(0);  
  }
}

void UserInterface::handle_wallet_creation(void) {
  if (!handle_seed_generation_config()) {
    state = MAIN_MENU;
    return;
  }
  bytes_data mnemonic = wallet.prepare_mnemonic(config);
  cli::display_mnemonic(mnemonic);
  cli::confirm_liability_waiver();

  const std::vector<uint32_t> PATH_DERIVE =
      Key_Derive::parse_derive_path(config.derivation_path);
  wallet.finalize_from_mnemonic(mnemonic, config.passphrase, PATH_DERIVE);
  state = WALLET_UI;
}

void UserInterface::handle_wallet_import(void) {
  std::string mnemonic = cli::request_input_mnemonic();
  if(mnemonic.empty()) {
    state = MAIN_MENU;
    return;
  }
  while (!wallet.correct_mnemonic(mnemonic)) {
    cli::incorrect_mnemonic_text();
    mnemonic = cli::request_input_mnemonic();

    if(mnemonic.empty()) {
    state = MAIN_MENU;
    return;
  }
  }

  std::string passphrase = cli::request_input_optional_passphrase();
  wallet.import_wallet(mnemonic, passphrase); // clears mnemonic and passphrase
  state = WALLET_UI;
}

bool UserInterface::handle_seed_generation_config(void) {
  char choice = cli::render_config_menu(config);

  while (choice != 'b' && choice != 'g') {

    if (choice == '1') {
      config.handle_extra_entropy();
    } else if (choice == '2') {
      config.handle_bit_length();
    } else if (choice == '3') {
      config.handle_use_passphrase();
    } else if (choice == '4') {
      config.handle_derivation_path();
    }

    choice = cli::render_config_menu(config);
  }

  return choice == 'g';
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
  if(cli::confirm_danger_action()) {
    cli::display_private_key(wallet.get_private_key());
  }
    break;
  case 5: // exit
    wallet.save();
    state = EXIT;
    break;
  }
}