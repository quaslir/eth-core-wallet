#pragma once
#include "core/secure_bytes_data.hpp"
#include "core/supported_networks.hpp"
#include "drivers/balance_client.hpp"
#include "drivers/gas_client.hpp"
#include "drivers/history_client.hpp"
#include "drivers/price_client.hpp"
#include <cstddef>
#include <string>
#include <vector>

class BlockchainClient {
public:
  BlockchainClient();
  std::function<secure_string(void)> get_current_eth_addr;
  void change_network(const networks::NetworkConfig &new_network);

  std::string get_active_network_name(void) const;
  std::string form_url(void) const;

  void update(void);
  assets_data get_balance(void) const;
  std::pair<std::vector<TransactionRecord>, bool>
  get_transaction_history(void) const;
  std::pair<double, bool> get_eth_price(void) const;
  std::pair<double, bool> get_current_gas(void) const;

  void update_history_manager(bool force = false);
  void update_price_manager(bool force = false);
  void update_balance_manager(bool force = false);
  void update_gas_manager(bool force = false);

private:
  HistoryManager history_manager;
  PriceManager price_manager;
  BalanceManager balance_manager;
  GasManager gas_manager;
  // uint32_t chain_id;
  networks::NetworkConfig active_network =
      networks::NetworkConfig{" 🌐 Ethereum Mainnet ", "eth-mainnet", 1};
};
