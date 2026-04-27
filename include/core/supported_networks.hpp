#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace networks {
struct NetworkConfig {
  std::string name;
  std::string rpc_prefix;
  uint32_t chain_id;
};

const static std::vector<NetworkConfig> list = {
    {" 🌐 Ethereum Mainnet ", "eth-mainnet", 1},
    {" 💜 Polygon PoS      ", "polygon-mainnet", 137},
    {" 💙 Arbitrum One     ", "arb-mainnet", 42161},
    {" 🔴 Optimism         ", "opt-mainnet", 10},
    {" 🛡️ Base             ", "base-mainnet", 8453},
    {" 🧪 Sepolia Testnet  ", "eth-sepolia", 11155111}};

inline std::vector<std::string> get_network_names(void) {
  std::vector<std::string> names;

  for (const auto &net : list) {
    names.push_back(net.name);
  }

  return names;
}
} // namespace networks
