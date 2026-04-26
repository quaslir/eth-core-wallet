#include "async_eth_usd_update.hpp"
#include "async_manager.hpp"
#include "async_transactions_history_manager.hpp"
#include "blockchain_client.hpp"
#include "cli.hpp"
#include "config.hpp"
#include "iwallet_actions.hpp"
#include "json.hpp"
#include "wallet.hpp"
#include <cstddef>
#include <string>
struct TEMP_DATA {
  bytes_data password_for_wallet_unlocking;
  bytes_data mnemonic, passphrase;
};

class UserInterface : public IWalletActions {
public:
  void load(void);

private:
  Wallet wallet;
  Config config;
  BlockchainClient block_client;
  AsyncBalanceManager balance_manager = AsyncBalanceManager(block_client);
  AsyncTransactionsHistoryManager history_manager =
      AsyncTransactionsHistoryManager(block_client);
  Async_ETH_USD_Currency eth_price_manager;
  TEMP_DATA temp;
  CLI cli;

  void apply_choice_from_wallet_ui(int choice) override;

  bytes_data get_mnemonic(void) override;
  const Config &get_config(void) override;
  const Wallet &get_wallet(void) override;
  void on_main_menu(int choice) override;
  void add_passphrase(const bytes_data &passphrase) override;
  void set_password_for_wallet(bytes_data password) override;
  bytes_data get_password_for_wallet(void) override;
  bool check_mnemonic(std::string_view mnemonic) override;
  void set_mnemonic(std::string_view mnemonic) override;
  void set_passphrase(std::string_view passphrase) override;
  void import_wallet(void) override;
  bool check_password(bytes_data &password) override;
  void load_wallet(void) override;
  void save_wallet(void) override;
  void change_bit_length(int new_bit_length) override;
  void set_extra_entropy(std::string_view entropy) override;
  void change_derivation_path(std::string_view derive_path) override;
  void create_wallet(void) override;
  void update_balance(void) override;
  void copy_address(void) override;
  void copy_private_key(void) override;
  const bytes_data &get_private_key(void) override;
  std::vector<TransactionRecord> get_transactions_history(void) override;
  void update_transactions_data(void) override;
  void request_transactions_data(void) override;
  std::string get_current_network(void) override;
  void change_network(size_t index) override;
  void update_eth_price(void) override;
  double get_current_eth_price(void) override;
};
