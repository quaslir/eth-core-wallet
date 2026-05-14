#pragma once
#include "core/asset.hpp"
#include "core/secure_bytes_data.hpp"
#include <cstdint>
#include <map>
#include <string>
struct WalletInfo {
  secure_string addr{};
  std::map<std::pair<uint64_t, std::string>, Asset> assets{};
  double total = 0.0;
};
