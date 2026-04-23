#pragma once
#include "json.hpp"
#include "supported_networks.hpp"
#include <string>
#include <vector>
struct TransactionRecord {
  std::string hash;
  double value;
  std::string asset;
  std::string from;
  std::string to;
  std::string timestamp;
  bool incoming;
};

class BlockchainClient {
public:
  std::string get_balance(const std::string &eth_addr) const;
  std::vector<TransactionRecord>
  get_transaction_history(const std::string &eth_addr) const;
  BlockchainClient() : active_network(networks::list[0]) {}
  void change_network(const networks::NetworkConfig &new_network);

  std::string get_active_network_name(void) const;

private:
  static std::vector<TransactionRecord>
  parse_transactions(const json &j, bool incoming = true);
  std::string form_url(void) const;
  // uint32_t chain_id;
  networks::NetworkConfig active_network;
};
