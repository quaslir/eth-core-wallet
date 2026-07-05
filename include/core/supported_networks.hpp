#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace networks {
struct NetworkConfig {
  std::string name;
  std::string rpc_prefix;
  uint32_t chain_id;
  std::string explorer_api_url;
};

const static std::vector<NetworkConfig> list = {
    {" 🌐 Ethereum Mainnet ", "eth-mainnet", 1, "https://api.etherscan.io/api"},
    {" 💜 Polygon PoS      ", "polygon-mainnet", 137,
     "https://api.polygonscan.com/api"},
    {" 💙 Arbitrum One     ", "arb-mainnet", 42161,
     "https://api.arbiscan.io/api"},
    {" 🔴 Optimism         ", "opt-mainnet", 10,
     "https://api-optimistic.etherscan.io/api"},
    {" 🧪 Sepolia Testnet  ", "eth-sepolia", 11155111,
     "https://api-sepolia.etherscan.io/api"}};
inline std::vector<std::string> get_network_names(void) {
  std::vector<std::string> names;

  for (const auto &net : list) {
    names.push_back(net.name);
  }

  return names;
}

inline std::string get_coingecko_platform_id(uint64_t chain_id) {
  switch (chain_id) {
  case 1:
    return "ethereum";
  case 137:
    return "polygon-pos";
  case 42161:
    return "arbitrum-one";
  case 10:
    return "optimistic-ethereum";
  case 11155111:
    return "";
  default:
    return "";
  }
} // namespace networks
} // namespace networks
