#pragma once
#include "core/secure_bytes_data.hpp"
#include "core/supported_networks.hpp"
#include "drivers/balance_client.hpp"
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
  double get_balance(void) const;
  std::vector<TransactionRecord> get_transaction_history(void) const;
  double get_eth_price(void) const;


  void update_history_manager(void);
  void update_price_manager(void);
  void update_balance_manager(void);
private:
HistoryManager history_manager;
PriceManager price_manager;
BalanceManager balance_manager;
  // uint32_t chain_id;
  networks::NetworkConfig active_network;
};
