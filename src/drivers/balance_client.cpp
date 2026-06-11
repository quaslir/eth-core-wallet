#include "drivers/balance_client.hpp"
#include "api/http.hpp"
#include "api/json.hpp"
#include "config/configuration.hpp"
#include "core/asset.hpp"
#include "core/secure_bytes_data.hpp"
#include "core/uint256.hpp"
#include "drivers/price_client.hpp"
#include "utils/tech_utils.hpp"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <future>
#include <memory>
#include <string>
#include <utility>
#include <iostream>
std::string form_data(const std::string &contract_address,
                      const secure_string &eth_addr) {
  json j;
  j["jsonrpc"] = "2.0";
  j["id"] = 1;
  j["method"] = "eth_call";

  json tx_params;

  tx_params["to"] = contract_address;
  tx_params["data"] = "0x70a08231000000000000000000000000" + eth_addr.substr(2);

  j["params"] = json::array({tx_params, "latest"});

  return j.dump();
}
bool BalanceManager::update_one_asset(Asset &asset,
                                      const secure_string &eth_addr) const {
  try {
    std::string data = form_data(asset.contract_address, eth_addr);

    std::string buffer = http::post_request(form_url(), data);

    json j = json::parse(buffer);

    std::string res = j.at("result").get<std::string>();

    Uint256 res_val(res, true);

    std::string converted = res_val.from_wei_to_asset(
        tech_utils::decimals_to_divisor(asset.decimals));

    double value = 0.0;

    if (!tech_utils::to_double(converted, value))
      return false;

    asset.balance = value;

    return true;

  } catch (const std::exception &err) {
    return false;
  }
}

assets_data BalanceManager::update_all(const secure_string &eth_addr) const {
  assets_data new_assets = crypto_assets::get_default_assets();

  std::vector<std::future<bool>> balance_futures;

  balance_futures.reserve(new_assets.size());

  for (auto &[key, asset] : new_assets) {
    Asset *asset_ptr = &asset;
    if (asset.is_native) {
      balance_futures.push_back(
          std::async(std::launch::async, [this, asset_ptr, &eth_addr]() {
            return update_native(*asset_ptr, eth_addr);
          }));

    } else
      balance_futures.push_back(
          std::async(std::launch::async, [this, asset_ptr, &eth_addr]() {
            return update_one_asset(*asset_ptr, eth_addr);
          }));
  }

  std::vector<std::string> symbols;

  symbols.reserve(new_assets.size());

  for (const auto &asset : new_assets) {
    symbols.push_back(asset.second.id);
  }

  auto price_future = std::async(std::launch::async, [&symbols]() {
    return price_manager::request_prices(symbols);
  });

  for (auto &f : balance_futures)
    f.wait();

  auto prices = price_future.get();

  for (auto &asset : new_assets) {
    auto it = prices.find(asset.second.id);

    if (it != prices.end()) {
      asset.second.fiat_price = it->second;
    }
  }

  return new_assets;
}

bool BalanceManager::update_native(Asset &asset,
                                   const secure_string &eth_addr) const {

  try {
    AlchemyJSON alchm("2.0", "eth_getBalance",
                      {std::string{eth_addr}, "latest"}, 1);

    std::string data = alchm.to_string();

    std::string buffer = http::post_request(form_url(), data);
    if (buffer.empty())
      return false;

    alchm.parse(buffer);

    Uint256 uint256_t(alchm.get_result(), 1);
    std::string divisor = tech_utils::decimals_to_divisor(asset.decimals);
    std::string res = uint256_t.from_wei_to_asset(divisor);

    double value = 0.0;

    if (!tech_utils::to_double(res, value))
      return false;
    asset.balance = value;
    return true;

  } catch (const std::exception &err) {
    return false;
  }
}

void BalanceManager::request(const secure_string &addr) {
  if (!can_request())
    return;
  updating = true;
  uint64_t gen = get_generation();
  worker = std::async(std::launch::async, [this, addr, gen]() {
    return std::make_pair(update_all(addr), gen);
  });
}

void BalanceManager::update(void) {
  if (updating && worker.valid()) {
    auto status = worker.wait_for(std::chrono::milliseconds(0));

    if (status == std::future_status::ready) {
      try {
        auto [assets, gen] = worker.get();
        if (gen == get_generation()) {
          auto ptr = std::make_shared<assets_data>(std::move(assets));

#if defined(__cpp_lib_atomic_shared_ptr) &&                                    \
    __cpp_lib_atomic_shared_ptr >= 201711L
          atomic_assets.store(ptr);
#else
          std::atomic_store(&atomic_assets, ptr);
#endif
        }

      } catch (...) {
      }
      updating = false;
      last_update_time = std::chrono::steady_clock::now();
    }
  }
}
std::shared_ptr<assets_data> BalanceManager::get_balance() const {
#if defined(__cpp_lib_atomic_shared_ptr) &&                                    \
    __cpp_lib_atomic_shared_ptr >= 201711L
  return atomic_assets.load();
#else
  return std::atomic_load(&atomic_assets);
#endif
}

BalanceManager::~BalanceManager() {
  if (worker.valid()) {
    worker.wait();
  }
}
