#include "core/assets.hpp"
#include "api/json.hpp"
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
