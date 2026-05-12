#pragma once
#include "core/secure_bytes_data.hpp"
#include "core/asset.hpp"
#include <map>
#include <string>
#include <cstdint>
struct WalletInfo {
  secure_string addr;
  std::map<std::pair<uint64_t, std::string>, Asset> assets;
};
