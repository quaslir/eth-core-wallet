#pragma once
#include "core/secure_bytes_data.hpp"
#include "core/wallet_info.hpp"
#include "drivers/blockchain_client.hpp"
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
struct TransactionRecord;

struct Config;

class IWalletActions {
public:
  virtual ~IWalletActions() = default;
  virtual secure_string get_mnemonic(void) = 0;
  virtual const Config &get_config(void) = 0;
  virtual WalletInfo get_wallet(void) = 0;
  virtual void on_main_menu(int choice) = 0;
  virtual void set_password_for_wallet(const secure_string &password) = 0;
  virtual secure_string get_password_for_wallet(void) = 0;
  virtual bool check_mnemonic(const secure_string &mnemonic) = 0;
  virtual void set_mnemonic(secure_string &&mnemonic) = 0;
  virtual void set_passphrase(secure_string &&passphrase) = 0;
  virtual void import_wallet(void) = 0;
  virtual bool check_password(const secure_string &password) = 0;
  virtual void load_wallet(void) = 0;
  virtual void save_wallet(void) = 0;
  virtual void change_bit_length(int new_bit_length) = 0;
  virtual void set_extra_entropy(bytes_data &&extra_entropy) = 0;
  virtual void add_passphrase(secure_string &&passphrase) = 0;
  virtual void change_derivation_path(secure_string &&derive_path) = 0;
  virtual void create_wallet(void) = 0;
  virtual void update_balance(void) = 0;
  virtual void copy_address(void) = 0;
  virtual void copy_private_key(void) = 0;
  virtual void apply_choice_from_wallet_ui(int choice) = 0;
  virtual const bytes_data &get_private_key(void) = 0;
  virtual std::vector<TransactionRecord> get_transactions_history(void) = 0;
  virtual void request_transactions_data(void) = 0;
  virtual void update_transactions_data(void) = 0;
  virtual std::string get_current_network(void) = 0;
  virtual void change_network(size_t index) = 0;
  virtual void update_eth_price(void) = 0;
  virtual double get_current_eth_price(void) = 0;
};
