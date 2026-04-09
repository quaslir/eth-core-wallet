#include "json.hpp"
#include "uint256.hpp"
#include <string>

class BlockchainClient {
public:
  std::string get_balance(const std::string &eth_addr) const;
};