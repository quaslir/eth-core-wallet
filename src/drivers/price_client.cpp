#include "drivers/price_client.hpp"
#include "api/http.hpp"
#include "api/json.hpp"
#include "config/configuration.hpp"
#include "core/secure_bytes_data.hpp"

void PriceManager::request(const secure_string &) {
  if (!can_request())
    return;

  updating = true;

  worker =
      std::async(std::launch::async, [this]() { return request_eth_price(); });
}

void PriceManager::update(void) {
  if (updating && worker.valid()) {
    auto status = worker.wait_for(std::chrono::milliseconds(0));

    if (status == std::future_status::ready) {
      try {
        current_price = worker.get();
      } catch (const std::exception &err) {
        current_price = 0.0;
      }
      updating = false;
      last_update_time = std::chrono::steady_clock::now();
    }
  }
}

double PriceManager::get_current_eth_price(void) const {
  return this->current_price;
}

double PriceManager::request_eth_price(void) const {
  try {
    std::string buffer = http::get_request(ETH_USD_URL);
    json j = json::parse(buffer);

    double value = j.value("USD", 0.0);
    return value;

  } catch (const std::exception &err) {
    return 0.0;
  }
}
