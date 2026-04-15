#include "ui.hpp"
#include "async_manager.hpp"

#include "config.hpp"
#include "iwallet_actions.hpp"
#include "json.hpp"
#include "security.hpp"
#include <string>
#include <string_view>

void UserInterface::load(void) {
cli.set_actions(this);
  EncryptedKeystore encrp;
  if (encrp.load()) {
    cli.set_active_tab(UNLOCK_PASSWORD);
  }

  cli.load();
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

    break;
  case 5: // exit
    // wallet.save();

    break;
  }
}

const Config& UserInterface::get_config(void) {
return this->config;
}

std::string UserInterface::get_mnemonic(void) {
    return std::string{temp.mnemonic.begin(), temp.mnemonic.end()};
}

void UserInterface::handle_config_menu(int choice) {
    if (choice == 1) {
      config.handle_extra_entropy();
    } else if (choice == 2) {
      cli.set_active_tab(BIT_LENGTH_CONFIG);
    } else if (choice == 3) {
      config.handle_use_passphrase();
    } else if (choice == 4) {
      config.handle_derivation_path();
    } else if (choice == 5) {
      cli.set_active_tab(MAIN_MENU);
    }

    else if (choice == 6) {
      temp.mnemonic = wallet.prepare_mnemonic(config);
      std::string view_mnemonic(reinterpret_cast<const char *>(temp.mnemonic.data()),
                                temp.mnemonic.size());

      cli.set_active_tab(MNEMONIC_DISPLAY);

      const std::vector<uint32_t> PATH_DERIVE =
          Key_Derive::parse_derive_path(config.derivation_path);

      wallet.finalize_from_mnemonic(temp.mnemonic, config.passphrase, PATH_DERIVE);
    }
}

 const Wallet& UserInterface::get_wallet(void) {
     return this->wallet;
 }
 void UserInterface::on_main_menu(int choice) {
     switch (choice) {

     case 1:
       cli.set_active_tab(CONFIG_MENU);
       break;
     case 2:
       cli.set_active_tab(IMPORT_MENU);
       // handle_wallet_import();
       break;
     case 3:
       exit(0);
     }
 }

 void UserInterface::set_password_for_wallet(bytes_data& password) {
this->temp.password_for_wallet_unlocking = password;
 }
bytes_data UserInterface::get_password_for_wallet(void) {
       return this->temp.password_for_wallet_unlocking;
}

bool UserInterface::check_mnemonic(std::string_view mnemonic) {
 return wallet.correct_mnemonic(mnemonic);
}



void UserInterface::set_mnemonic(std::string_view mnemonic) {
    temp.mnemonic = bytes_data(mnemonic.begin(), mnemonic.end());
}

void UserInterface::set_passphrase(std::string_view passphrase) {
    temp.passphrase = bytes_data(passphrase.begin(), passphrase.end());
}

void UserInterface::import_wallet(void) {
    wallet.import_wallet(temp.mnemonic, temp.passphrase);
}

bool UserInterface::check_password(bytes_data& password) {
    return security_manager::load_wallet(wallet, password);
}
void UserInterface::load_wallet(void) {
    std::vector<uint32_t> current_path =
        crypto_utils::change_derive_path(wallet.getIndex());
    wallet.sync_derive_path(current_path);
    wallet.derive(current_path);
}

void UserInterface::save_wallet(void) {
     wallet.save(temp.password_for_wallet_unlocking);
}

void UserInterface::change_bit_length(int new_bit_length){
    config.set_bit_length(new_bit_length);
}
void UserInterface::set_extra_entropy(std::string_view entropy) {
    config.set_extra_entropy(entropy);
}
