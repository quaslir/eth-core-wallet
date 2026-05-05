#include "drivers/price_client.hpp"
#include "config/configuration.hpp"
#include "api/http.hpp"
#include "api/json.hpp"
void PriceManager::request_eth_price(void) {

  if (updating)
    return;

  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now - last_update_time)
                     .count();
  if (elapsed < ETH_PRICE_TIMEOUT)
    return;

  updating = true;

  worker = std::async(std::launch::async, [this]() {
    return get_eth_price_in_usd();
  });
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

bool PriceManager::get_status(void) const { return this->updating; }

double PriceManager::get_eth_price_in_usd(void) const {
  try {
    std::string buffer = http::get_request(ETH_USD_URL);
    json j = json::parse(buffer);

    double value = j.value("USD", 0.0);
    return value;

  } catch (const std::exception &err) {
    return 0.0;
  }
}
