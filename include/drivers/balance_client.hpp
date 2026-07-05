#pragma once
#include "config/configuration.hpp"
#include "core/assets.hpp"
#include "core/secure_bytes_data.hpp"
#include "drivers/manager.hpp"
#include <atomic>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <string>

using assets_data = std::vector<Asset>;

class BalanceManager : public Manager {
private:
  std::future<std::pair<assets_data, uint64_t>> worker;
#if defined(__cpp_lib_atomic_shared_ptr) &&                                    \
    __cpp_lib_atomic_shared_ptr >= 201711L
  std::atomic<std::shared_ptr<assets_data>> atomic_assets{
      std::make_shared<assets_data>()};
#else
  std::shared_ptr<assets_data> atomic_assets{std::make_shared<assets_data>()};
#endif
  bool update_native(Asset &asset, const secure_string &eth_addr) const;
  bool update_one_asset(Asset &asset, const secure_string &eth_addr) const;
  assets_data update_all(const secure_string &eth_addr) const;

  std::function<uint64_t()> get_current_chain_id;
  std::function<assets_data(uint64_t)> get_current_assets;

public:
  std::function<std::string(void)> form_url;
  BalanceManager()
      : Manager(Configuration::get_instance().BALANCE_TIMEOUT),
        atomic_assets(std::make_shared<assets_data>(
            crypto_assets::get_default_assets())) {}
  ~BalanceManager();
  void request(const secure_string &eth_addr) override;
  void update(void) override;
  std::shared_ptr<assets_data> get_balance(void) const;
  void set_current_chain_id_callback(std::function<uint64_t()> callback);
  void
  set_current_assets_callback(std::function<assets_data(uint64_t)> callback);
};
