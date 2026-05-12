#include "drivers/balance_client.hpp"
#include "api/http.hpp"
#include "api/json.hpp"
#include "config/configuration.hpp"
#include "core/asset.hpp"
#include "core/secure_bytes_data.hpp"
#include "core/uint256.hpp"
#include "utils/tech_utils.hpp"
#include <chrono>
#include <exception>
#include <string>
#include <iostream>

std::string form_data(const std::string& contract_address, const secure_string& eth_addr) {
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
bool BalanceManager::update_one_asset(Asset& asset, const secure_string& eth_addr) const {
try {
    std::string data = form_data(asset.contract_address, eth_addr);

    std::string buffer = http::post_request(form_url(),data);

    json j = json::parse(buffer);

    std::string res = j.at("result").get<std::string>();

    Uint256 res_val(res, true);

    std::string converted = res_val.from_wei_to_asset(tech_utils::decimals_to_divisor(asset.decimals));

    double value = 0.0;

    if(!tech_utils::to_double(converted, value)) return false;

    asset.balance = value;

    return true;

} catch(const std::exception& err) {
    return false;
}
}



assets_data BalanceManager::update_all(const secure_string& eth_addr) const {
    assets_data new_assets = crypto_assets::get_default_assets();
    for(auto & asset : new_assets) {
        if(asset.second.is_native) {
            update_native(asset.second, eth_addr);
        } else
        update_one_asset(asset.second, eth_addr);

    }

    return new_assets;
}



bool BalanceManager::update_native(Asset & asset,const secure_string &eth_addr) const {

  try {
    AlchemyJSON alchm("2.0", "eth_getBalance",
                      {std::string{eth_addr}, "latest"}, 1);

    std::string data = alchm.to_string();

    std::string buffer = http::post_request(form_url(), data);
    if (buffer.empty())
        return false;

    alchm.parse(buffer);

    Uint256 uint256_t(alchm.get_result(), 1);

    std::string res = uint256_t.from_wei_to_asset(WEI_TO_ETH);

    double value = 0.0;

    if(!tech_utils::to_double(res, value)) return false;
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

  worker = std::async(std::launch::async,
                      [this, addr]() { return update_all(addr); });
}

void BalanceManager::update(void) {
  if (updating && worker.valid()) {
    auto status = worker.wait_for(std::chrono::milliseconds(0));

    if (status == std::future_status::ready) {
      try {
            assets = worker.get();

      } catch (...) {
      }
      updating = false;
      last_update_time = std::chrono::steady_clock::now();
    }
  }
}
assets_data BalanceManager::get_balance(void) const { return this->assets; }

void BalanceManager::clear_timer(void) {
  last_update_time = std::chrono::steady_clock::now() -
                     std::chrono::milliseconds(TRANSACTION_TIMEOUT);
}

void BalanceManager::clear(void) {
  updating = false;
}

BalanceManager::~BalanceManager() {
  if (worker.valid()) {
    worker.wait();
  }
}
