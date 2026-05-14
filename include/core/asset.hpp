
#pragma once
#include <cstdint>
#include <map>
#include <string>
struct Asset {
  std::string symbol;
  std::string name;

  std::string network_name;
  uint64_t chain_id;

  std::string contract_address;
  int decimals;

  double balance = 0.0;
  double fiat_price = 0.0;
  bool is_native;

  bool is_updating;
  bool has_error;
};

namespace crypto_assets {
inline std::map<std::pair<uint64_t, std::string>, Asset>
get_default_assets(void) {
  std::map<std::pair<uint64_t, std::string>, Asset> assets;
  auto add = [&](Asset a) { assets[{a.chain_id, a.contract_address}] = a; };

  add({"ETH", "Ethereum", "Sepolia", 11155111, "", 18, 0.0, 0.0, true, false,
       false});
  add({"WETH", "Wrapped ETH", "Sepolia", 11155111,
       "0xfFf9976782d46CC05630D1f6eBAb18b2324d6B14", 18, 0.0, 0.0, false, false,
       false});
  add({"USDC", "USD Coin", "Sepolia", 11155111,
       "0x1c7d4b196cb0c7b01d743fbc6116a902379c7238", 6, 0.0, 0.0, false, false,
       false});
  add({"LINK", "Chainlink", "Sepolia", 11155111,
       "0x779877A7B0D9E8603169DdbD7836e478b4624789", 18, 0.0, 0.0, false, false,
       false});

  return assets;
}
} // namespace crypto_assets
