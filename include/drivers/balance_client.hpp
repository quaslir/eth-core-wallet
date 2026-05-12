#pragma once
#include "config/configuration.hpp"
#include "core/asset.hpp"
#include "core/secure_bytes_data.hpp"
#include "drivers/manager.hpp"
#include <cstdint>
#include <functional>
#include <future>
#include <string>

using assets_data = std::map<std::pair<uint64_t, std::string>, Asset>;

class BalanceManager : public Manager {
private:
  std::future<assets_data> worker;
  assets_data assets;
  bool update_native(Asset & asset,const secure_string &eth_addr) const;
  bool update_one_asset(Asset& asset, const secure_string& eth_addr) const;
  assets_data update_all(const secure_string& eth_addr) const;

public:
  std::function<std::string(void)> form_url;
  BalanceManager() : Manager(BALANCE_TIMEOUT) {}
  ~BalanceManager();
  void request(const secure_string &eth_addr) override;
  void update(void) override;
  assets_data get_balance(void) const;
  void clear(void);
  void clear_timer(void);
};
