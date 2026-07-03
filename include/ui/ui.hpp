#include "cli.hpp"
#include "config/config.hpp"
#include "core/assets.hpp"
#include "core/secure_bytes_data.hpp"
#include "core/wallet.hpp"
#include "core/wallet_info.hpp"
#include "drivers/blockchain_client.hpp"
#include "drivers/rpc_bridge.hpp"
#include "iwallet_actions.hpp"
class UserInterface : public IWalletActions {
public:
  void load(void);

private:
  Wallet wallet;
  Config config;
  BlockchainClient block_client;
  struct TEMP_DATA {
    secure_string password_for_wallet_unlocking;
    secure_string mnemonic, passphrase;
  };
  TEMP_DATA temp;
  CLI cli;
  AssetsStore assets_store;
  RpcBridge rpc_bridge;
  std::shared_ptr<DappRequest> pending_request;
  void apply_choice_from_wallet_ui(int choice) override;

  secure_string get_mnemonic(void) override;
  const Config &get_config(void) override;
  WalletInfo get_wallet(void) override;
  void on_main_menu(int choice) override;
  void add_passphrase(secure_string &&passphrase) override;
  void set_password_for_wallet(const secure_string &password) override;
  secure_string get_password_for_wallet(void) override;
  bool check_mnemonic(const secure_string &mnemonic) override;
  void set_mnemonic(secure_string &&mnemonic) override;
  void set_passphrase(secure_string &&passphrase) override;
  void import_wallet(void) override;
  bool check_and_load_wallet(const secure_string &password) override;
  void load_wallet(void) override;
  void save_wallet(void) override;
  void change_bit_length(int new_bit_length) override;
  void set_extra_entropy(bytes_data &&extra_entropy) override;
  void change_derivation_path(secure_string &&derive_path) override;
  void create_wallet(void) override;
  void update_balance(bool force = false) override;
  void copy_address(void) override;
  void copy_private_key(void) override;
  void copy_mnemonic(void) override;
  const bytes_data &get_private_key(void) override;
  std::pair<std::shared_ptr<std::vector<TransactionRecord>>, bool>
  get_transactions_history(void) override;
  void update_transactions_data(bool force = false) override;
  std::string get_current_network(void) override;
  void change_network(size_t index) override;
  void wipe_mnemonic(void) override;
  std::pair<double, bool> get_current_gas_price(void) override;
  float get_next_refresh(void) override;
  void update_gas_price(bool force = false) override;
  void update_info(void) override;
  const std::deque<ActivityEvent> &get_activity(void) override;
  std::pair<std::string, bool>
  send_transaction(const std::string &to, const Asset &asset,
                   const std::string &amount, double target_gas_gwei,
                   const std::string &gas_limit_input) override;
  std::pair<TxStatus, bool> get_current_tx_status(void) override;
  void update_current_tx_status(void) override;
  bool speed_up_transaction(void) override;
  bool cancel_transaction(void) override;
  bool check_password(const secure_string &password) override;

  uint64_t get_current_chain_id(void) override;
  std::shared_ptr<DappRequest> get_pending_dapp_request(void) override;
  void approve_dapp_request(uint64_t id) override;
  void reject_dapp_request(uint64_t id) override;
  void toggle_dapp_bridge(bool enable) override;
  bool is_bridge_running(void) override;
  void add_new_asset(const Asset &new_asset) override;
  Asset fetch_new_asset_metadata(const std::string &contract_addr) override;
};
