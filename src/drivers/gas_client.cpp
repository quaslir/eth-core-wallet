#include "drivers/gas_client.hpp"
#include "api/http.hpp"
#include "api/json.hpp"
#include "config/configuration.hpp"
#include "core/uint256.hpp"
#include "utils/tech_utils.hpp"
#include <cstdlib>
#include <exception>
#include <future>
#include <string>
double GasManager::get_current_gas(void) const { return current_gas_price; }

double GasManager::request_gas(void) const {

  AlchemyJSON alchm("2.0", "eth_gasPrice", {}, 1);
  std::string data = alchm.to_string();
  std::string buffer = http::post_request(form_url(), data);
  json j = json::parse(buffer);
  std::string res = j.at("result").get<std::string>();
  Uint256 value(res, true);
  std::string converted = value.from_wei_to_asset(WEI_TO_GWEI);

  double to_number;
  if (!tech_utils::to_double(converted, to_number))
    return 0.0;

  return to_number;

  return 0.0;
}

void GasManager::request(const secure_string &) {
  if (!can_request())
    return;

  updating = true;

  worker = std::async(std::launch::async, [this]() { return request_gas(); });
}
void GasManager::update(void) {
  if (updating && worker.valid()) {
    auto status = worker.wait_for(std::chrono::milliseconds(0));

    if (status == std::future_status::ready) {
      try {
        current_gas_price = worker.get();
      } catch (const std::exception &err) {
        current_gas_price = 0.0;
      }
      updating = false;
      last_update_time = std::chrono::steady_clock::now();
    }
  }
}
