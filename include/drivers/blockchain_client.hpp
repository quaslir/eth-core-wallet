#pragma once
#include "core/asset.hpp"
#include "core/secure_bytes_data.hpp"
#include "core/supported_networks.hpp"
#include "drivers/balance_client.hpp"
#include "drivers/gas_client.hpp"
#include "drivers/history_client.hpp"
#include "drivers/price_client.hpp"
#include "drivers/transaction_client.hpp"
#include <chrono>
#include <cstddef>
#include <deque>
#include <memory>
#include <string>
#include <vector>

struct ActivityEvent {
  std::string icon;
  std::string msg;
  std::chrono::system_clock::time_point time;
};

class BlockchainClient {
public:
  BlockchainClient();
  std::function<secure_string(void)> get_current_eth_addr;
  void change_network(const networks::NetworkConfig &new_network);

  std::string get_active_network_name(void) const;
  std::string form_url(void) const;

  void update(void);
  std::shared_ptr<assets_data> get_balance(void) const;
  std::pair<std::vector<TransactionRecord>, bool>
  get_transaction_history(void) const;
  std::pair<double, bool> get_current_gas(void) const;

  bool update_history_manager(bool force = false);
  bool update_balance_manager(bool force = false);
  bool update_gas_manager(bool force = false);
  float get_next_refresh(void) const;
  void push_activity(const std::string &icon, const std::string &msg);
  const std::deque<ActivityEvent> &get_activity(void) const;
  bool send_raw_transaction(const secure_string &to_addr,
                            const bytes_data &private_key, const Asset &asset,
                            const std::string &value);

private:
  HistoryManager history_manager;
  BalanceManager balance_manager;
  GasManager gas_manager;
  TransactionManager transaction_manager;
  std::chrono::steady_clock::time_point last_update_time;
  std::deque<ActivityEvent> activity_log;
  networks::NetworkConfig active_network =
      networks::NetworkConfig{" 🌐 Ethereum Mainnet ", "eth-mainnet", 1};
};
