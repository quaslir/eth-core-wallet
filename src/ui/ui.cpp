#include "ui/ui.hpp"

#include "api/json.hpp"
#include "config/config.hpp"
#include "core/secure_bytes_data.hpp"
#include "core/security.hpp"
#include "drivers/blockchain_client.hpp"
#include "iwallet_actions.hpp"
#include "utils/tech_utils.hpp"
#include <cstddef>
#include <cstdlib>
#include <openssl/crypto.h>
#include <string>

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
    request_transactions_data();
    cli.set_active_tab(TRANSACTION_HISTORY);
    break;
  case 3:
    cli.set_active_tab(CHANGE_NETWORK);
    break;
  case 4:
    wallet.derive_next();
    balance_manager.clear();
    break;

  case 5:
    if (wallet.derive_prev()) {

      balance_manager.clear_timer();
      wallet.set_balance(0.0);
      update_balance();
    }
    break;
  case 6: // show private_key
    cli.set_active_tab(DISPLAY_PRIVATE_KEY);
    break;
  case 7: // exit
    // wallet.save();

    break;
  }
}

const Config &UserInterface::get_config(void) { return this->config; }

secure_string UserInterface::get_mnemonic(void) { return temp.mnemonic; }

void UserInterface::create_wallet(void) {
  temp.mnemonic = wallet.prepare_mnemonic(config);
  cli.set_active_tab(MNEMONIC_DISPLAY);

  const std::vector<uint32_t> PATH_DERIVE =
      Key_Derive::parse_derive_path(config.derivation_path);

  wallet.finalize_from_mnemonic(temp.mnemonic, config.passphrase, PATH_DERIVE);
}

WalletInfo UserInterface::get_wallet(void) {
  return WalletInfo(wallet.get_eth_address(), balance_manager.get_balance());
}
void UserInterface::on_main_menu(int choice) {
  switch (choice) {

  case 0:
    cli.set_active_tab(CONFIG_MENU);
    break;
  case 1:
    cli.set_active_tab(IMPORT_MENU);
    // handle_wallet_import();
    break;
  case 2:
    exit(0);
  }
}

void UserInterface::set_password_for_wallet(const secure_string &password) {
  this->temp.password_for_wallet_unlocking = password;
}
secure_string UserInterface::get_password_for_wallet(void) {
  return this->temp.password_for_wallet_unlocking;
}

bool UserInterface::check_mnemonic(const secure_string &mnemonic) {
  return wallet.correct_mnemonic(mnemonic);
}

void UserInterface::set_mnemonic(secure_string &&mnemonic) {
  temp.mnemonic = std::move(mnemonic);
}

void UserInterface::set_passphrase(secure_string &&passphrase) {
  temp.passphrase = std::move(passphrase);
}

void UserInterface::import_wallet(void) {
  wallet.import_wallet(temp.mnemonic, temp.passphrase);
}

bool UserInterface::check_password(const secure_string &password) {
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

void UserInterface::change_bit_length(int new_bit_length) {
  config.set_bit_length(new_bit_length);
}
void UserInterface::set_extra_entropy(bytes_data &&entropy) {
  config.set_extra_entropy(std::move(entropy));
}

void UserInterface::add_passphrase(secure_string &&passphrase) {
  config.set_passphrase(std::move(passphrase));
}

void UserInterface::change_derivation_path(secure_string &&derive_path) {
  config.change_derivation_path(std::move(derive_path));
}
void UserInterface::update_balance(void) {
  if (!wallet.is_loaded())
    return;

  if (!balance_manager.get_status()) {
    wallet.set_balance(balance_manager.get_balance());

    balance_manager.request_balance(std::string{wallet.get_eth_address()});
  }

  balance_manager.update();
}

void UserInterface::update_eth_price(void) {
  if (!wallet.is_loaded())
    return;

  if (!eth_price_manager.get_status()) {
    eth_price_manager.request_eth_price();
  }

  eth_price_manager.update();
}

void UserInterface::copy_address(void) {
#ifdef __APPLE__
  secure_string command = "echo " + wallet.get_eth_address() + "| pbcopy";
  std::system(command.c_str());
#else
// handle
#endif
}

void UserInterface::copy_private_key(void) {

#ifdef __APPLE__
  secure_string command =
      "echo " + tech_utils::to_hex(wallet.get_private_key()) + "| pbcopy";
  std::system(command.c_str());
#else
// handle
#endif
}

std::vector<TransactionRecord> UserInterface::get_transactions_history(void) {
  return history_manager.get_transactions_history();
}

void UserInterface::request_transactions_data(void) {
  std::string addr{};
  if (history_manager.get_status())
    return;

  addr = wallet.get_eth_address();

  if (addr.empty())
    return;
  history_manager.request_transactions_data(addr);
}

void UserInterface::update_transactions_data(void) {
  if (!wallet.is_loaded())
    return;
  if (history_manager.get_status()) {
    history_manager.update();
  }
}

const bytes_data &UserInterface::get_private_key(void) {
  return wallet.get_private_key();
}
std::string UserInterface::get_current_network(void) {
  return block_client.get_active_network_name();
}

void UserInterface::change_network(size_t index) {
  if (index >= networks::list.size())
    return; // handle
  block_client.change_network(networks::list[index]);
}

double UserInterface::get_current_eth_price(void) {
  return eth_price_manager.get_current_eth_price();
}

void UserInterface::wipe_mnemonic(void) {
OPENSSL_cleanse(temp.mnemonic.data(), temp.mnemonic.size());
}