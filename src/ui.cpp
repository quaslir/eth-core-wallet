#include "ui.hpp"
#include "async_manager.hpp"

#include "blockchain_client.hpp"
#include "config.hpp"
#include "iwallet_actions.hpp"
#include "json.hpp"
#include "security.hpp"
#include "tech_utils.hpp"
#include <cstddef>
#include <cstdlib>
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

bytes_data UserInterface::get_mnemonic(void) { return temp.mnemonic; }

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

void UserInterface::set_password_for_wallet(bytes_data password) {
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

bool UserInterface::check_password(bytes_data &password) {
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
void UserInterface::set_extra_entropy(std::string_view entropy) {
  config.set_extra_entropy(entropy);
}

void UserInterface::add_passphrase(const bytes_data &pass) {
  config.set_passphrase(pass);
}

void UserInterface::change_derivation_path(std::string_view derive_path) {
  config.change_derivation_path(derive_path);
}
void UserInterface::update_balance(void) {
  if (!wallet.is_loaded())
    return;

  if (!balance_manager.get_status()) {
    wallet.set_balance(balance_manager.get_balance());

    balance_manager.request_balance(wallet.get_eth_address());
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
  std::string command =
      "echo " + wallet.get_eth_address() + "| pbcopy";
  std::system(command.c_str());
#else
// handle
#endif
}

void UserInterface::copy_private_key(void) {

#ifdef __APPLE__
  std::string command =
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
