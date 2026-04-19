#include "json.hpp"
#include "uint256.hpp"
#include <string>
#include <vector>
struct TransactionRecord {
    std::string hash;
    double value;
    std::string asset;
    std::string from_to;
    std::string timestamp;
    bool incoming;
};


class BlockchainClient {
public:
  std::string get_balance(const std::string &eth_addr) const;
  void get_transaction_history(const std::string& eth_addr) const;

  private:
  std::vector<TransactionRecord> parse_transactions(const json& j) const;
};
