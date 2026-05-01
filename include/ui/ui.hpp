#include "api/json.hpp"
#include "async-managers/async_eth_usd_update.hpp"
#include "async-managers/async_manager.hpp"
#include "async-managers/async_transactions_history_manager.hpp"
#include "cli.hpp"
#include "config/config.hpp"
#include "core/wallet.hpp"
#include "core/wallet_info.hpp"
#include "drivers/blockchain_client.hpp"
#include "iwallet_actions.hpp"
#include "core/secure_bytes_data.hpp"
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
  struct TEMP_DATA {
    secure_string password_for_wallet_unlocking;
    secure_string mnemonic, passphrase;
  };
  TEMP_DATA temp;
  CLI cli;

  void apply_choice_from_wallet_ui(int choice) override;

  secure_string get_mnemonic(void) override;
  const Config &get_config(void) override;
  WalletInfo get_wallet(void) override;
  void on_main_menu(int choice) override;
  void add_passphrase(secure_string &&passphrase) override;
  void set_password_for_wallet(const secure_string& password) override;
  secure_string get_password_for_wallet(void) override;
  bool check_mnemonic(const secure_string& mnemonic) override;
  void set_mnemonic(secure_string&& mnemonic) override;
  void set_passphrase(secure_string&& passphrase) override;
  void import_wallet(void) override;
  bool check_password(const secure_string& password) override;
  void load_wallet(void) override;
  void save_wallet(void) override;
  void change_bit_length(int new_bit_length) override;
  void set_extra_entropy(bytes_data&& extra_entropy) override;
  void change_derivation_path(secure_string&& derive_path) override;
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
