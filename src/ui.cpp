#include "ui.hpp"
#include "cli.hpp"
#include "config.hpp"
#include <iostream>
void UserInterface::load(void) {
  while (state != EXIT) {
    if (state == MAIN_MENU) {
      cli::print_welcome_message();
      int choice = cli::make_choice_from_welcome_message();
      apply_choice_from_welcome_message(choice);
    } else if (state == WALLET_UI) {
      cli::print_wallet_ui(wallet);
      char ch = cli::handle_wallet_ui_input();
      (void)ch;
    }
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
    break;
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

  wallet.finalize_from_mnemonic(mnemonic, config.passphrase);
  state = WALLET_UI;
}

void UserInterface::handle_wallet_import(void) {
  std::string mnemonic = cli::request_input_mnemonic();

  while (!wallet.correct_mnemonic(mnemonic)) {
    cli::incorrect_mnemonic_text();
    mnemonic = cli::request_input_mnemonic();
  }

  std::string passphrase = cli::request_input_optional_passphrase();
  wallet.import_wallet(mnemonic, passphrase);
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