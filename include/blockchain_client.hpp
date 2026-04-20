#pragma once
#include "json.hpp"
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
  static std::vector<TransactionRecord> get_transaction_history(const std::string& eth_addr);

  private:
 static  std::vector<TransactionRecord> parse_transactions(const json& j, bool incoming = true);
};
