
#pragma once
#include "api/json.hpp"
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
struct Asset {
  std::string symbol{};
  std::string name{};

  std::string network_name{};
  uint64_t chain_id{};

  std::string contract_address{};
  int decimals{};

  double balance = 0.0;
  double fiat_price = 0.0;
  bool is_native{};

  bool is_updating{};
  bool has_error{};

  std::string id{};

  bool operator==(const Asset &other) const {
    if (is_native && other.is_native) {
      return chain_id == other.chain_id;
    }

    return chain_id == other.chain_id &&
           contract_address == other.contract_address;
  }
};

inline void to_json(json &j, const Asset &a) {
  j = json{{"symbol", a.symbol},
           {"name", a.name},
           {"network_name", a.network_name},
           {"chain_id", a.chain_id},
           {"contract_address", a.contract_address},
           {"decimals", a.decimals},
           {"balance", a.balance},
           {"fiat_price", a.fiat_price},
           {"is_native", a.is_native},
           {"id", a.id}};
}
inline void from_json(const json &j, Asset &a) {
  j.at("symbol").get_to(a.symbol);
  j.at("name").get_to(a.name);
  j.at("network_name").get_to(a.network_name);
  j.at("chain_id").get_to(a.chain_id);
  j.at("contract_address").get_to(a.contract_address);
  j.at("decimals").get_to(a.decimals);
  j.at("is_native").get_to(a.is_native);
  j.at("id").get_to(a.id);

  a.balance = j.value("balance", 0.0);
  a.fiat_price = j.value("fiat_price", 0.0);
  a.is_updating = false;
  a.has_error = false;
}

class AssetsStore {
private:
  std::unordered_map<uint64_t, std::vector<Asset>> current_assets;
  bool contains(const Asset &target_asset) const;

public:
  void load(const std::string &filename = "assets.json");
  void save(const std::string &filename = "assets.json");
  void add_asset(const Asset &new_asset);
  std::vector<Asset> get_current_assets(uint64_t target_chain_id = 1);
};

namespace crypto_assets {
inline std::vector<Asset> get_default_assets(uint64_t target_chain_id = 1) {
  std::vector<Asset> assets;
  switch (target_chain_id) {
  case 11155111: {
    assets.reserve(2);
    assets.push_back({"ETH", "Ethereum", "Sepolia", 11155111, "", 18, 0.0, 0.0,
                      true, false, false, "ethereum"});
    assets.push_back({"USDC", "USD Coin", "Sepolia", 11155111,
                      "0x1c7d4b196cb0c7b01d743fbc6116a902379c7238", 6, 0.0, 0.0,
                      false, false, false, "usd-coin"});
    break;
  }
  case 1: {
    assets.reserve(2);
    assets.push_back({"ETH", "Ethereum", "Mainnet", 1, "", 18, 0.0, 0.0, true,
                      false, false, "ethereum"});
    assets.push_back({"USDT", "Tether", "Mainnet", 1,
                      "0xdac17f958d2ee523a2206206994597c13d831ec7", 6, 0.0, 0.0,
                      false, false, false, "tether"});
    break;
  }
  case 137: {
    assets.reserve(1);
    assets.push_back({"MATIC", "Polygon", "Polygon", 137, "", 18, 0.0, 0.0,
                      true, false, false, "matic-network"});
    break;
  }
  case 42161: {
    assets.reserve(2);

    assets.push_back({"ETH", "Ethereum", "Arbitrum", 42161, "", 18, 0.0, 0.0,
                      true, false, false, "ethereum"});
    assets.push_back({"USDC", "USD Coin", "Arbitrum", 42161,
                      "0xaf88d065e77c8cc2239327c5edb3a432268e5831", 6, 0.0, 0.0,
                      false, false, false, "usd-coin"});
    break;
  }
  default:
    break;
  }

  return assets;
}
} // namespace crypto_assets
