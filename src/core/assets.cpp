#include "core/assets.hpp"
#include "api/json.hpp"
#include "drivers/balance_client.hpp"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

void AssetsStore::load(const std::string &filename) {
  std::ifstream file(filename);
  if (file.is_open()) {

    try {
      json j;
      file >> j;
      current_assets =
          j.get<std::unordered_map<uint64_t, std::vector<Asset>>>();
      return;
    } catch (...) {
      current_assets.clear();
    }
  }

  current_assets[11155111] = crypto_assets::get_default_assets(11155111);
  current_assets[1] = crypto_assets::get_default_assets(1);
  current_assets[137] = crypto_assets::get_default_assets(137);
  current_assets[42161] = crypto_assets::get_default_assets(42161);

  save(filename);
}

void AssetsStore::save(const std::string &filename) {
  std::ofstream file(filename);

  if (!file.is_open())
    return;

  json j = current_assets;
  file << j.dump();
}

std::vector<Asset> AssetsStore::get_current_assets(uint64_t target_chain_id) {
  return current_assets[target_chain_id];
}

void AssetsStore::add_asset(const Asset &new_asset) {
  if (contains(new_asset))
    return;
  current_assets[new_asset.chain_id].push_back(new_asset);

  save();
}

bool AssetsStore::contains(const Asset &target_asset) const {
  auto it = current_assets.find(target_asset.chain_id);

  if (it == current_assets.end()) {
    return false;
  }

  const auto &list = it->second;

  auto res = std::find_if(
      list.begin(), list.end(),
      [&target_asset](const Asset &asset) { return target_asset == asset; });

  return res != list.end();
}
