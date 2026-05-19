#include "ui/ui.hpp"

#include "api/json.hpp"
#include "config/config.hpp"
#include "core/asset.hpp"
#include "core/secure_bytes_data.hpp"
#include "core/security.hpp"
#include "drivers/balance_client.hpp"
#include "drivers/blockchain_client.hpp"
#include "iwallet_actions.hpp"
#include "utils/tech_utils.hpp"
#include <cstddef>
#include <cstdlib>
#include <openssl/crypto.h>
#include <string>
#include <unistd.h>
#include <vector>

void UserInterface::load(void) {
  cli.set_actions(this);
  EncryptedKeystore encrp;
  block_client.get_current_eth_addr = [this]() -> secure_string {
    return wallet.get_eth_address();
  };
  if (encrp.load()) {
    cli.set_active_tab(UNLOCK_PASSWORD);
  }

  cli.load();
}

void UserInterface::apply_choice_from_wallet_ui(int choice) {
  switch (choice) {
  case 1:
    cli.set_active_tab(SEND_FUNDS);
    break;
  case 2:
    update_transactions_data();

    cli.set_active_tab(TRANSACTION_HISTORY);
    break;
  case 3:
    cli.set_active_tab(CHANGE_NETWORK);
    break;
  case 4:
    wallet.derive_next();
    update_balance(true);
    break;

  case 5:
    if (wallet.derive_prev()) {
      update_balance(true);
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
  auto assets = block_client.get_balance();
  return WalletInfo(wallet.get_eth_address(), assets,
                    tech_utils::calculate_total(*assets));
}
void UserInterface::on_main_menu(int choice) {
  switch (choice) {

  case 0:
    cli.set_active_tab(CONFIG_MENU);
    break;
  case 1:
    cli.set_active_tab(IMPORT_MENU);

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
void UserInterface::update_balance(bool force) {

  block_client.update_balance_manager(force);
}

void UserInterface::update_transactions_data(bool force) {

  block_client.update_history_manager(force);
}

void UserInterface::update_gas_price(bool force) {
  block_client.update_gas_manager(force);
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

std::pair<double, bool> UserInterface::get_current_gas_price(void) {
  return block_client.get_current_gas();
}

std::pair<std::vector<TransactionRecord>, bool>
UserInterface::get_transactions_history(void) {
  return block_client.get_transaction_history();
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

void UserInterface::wipe_mnemonic(void) {
  OPENSSL_cleanse(temp.mnemonic.data(), temp.mnemonic.size());
}

float UserInterface::get_next_refresh(void) {
  return block_client.get_next_refresh();
}
void UserInterface::update_info(void) { block_client.update(); }

const std::deque<ActivityEvent> &UserInterface::get_activity(void) {
  return block_client.get_activity();
}

bool UserInterface::send_transaction(const std::string& to, const Asset& asset, const std::string& amount, double target_gas_gwei) {
    return block_client.send_raw_transaction(secure_string{to}, wallet.get_private_key(),asset, amount, target_gas_gwei);
}
