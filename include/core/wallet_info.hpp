#pragma once
#include "core/secure_bytes_data.hpp"
struct WalletInfo {
  secure_string addr;
  std::pair<double, bool> balance;
};
