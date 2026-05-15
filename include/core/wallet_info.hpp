#pragma once
#include "core/asset.hpp"
#include "core/secure_bytes_data.hpp"
#include "drivers/balance_client.hpp"
#include <cstdint>
#include <map>
#include <string>
struct WalletInfo {
  secure_string addr{};
  std::shared_ptr<assets_data> assets{};
  double total = 0.0;
};
