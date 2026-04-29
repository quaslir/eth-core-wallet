#include "async-managers/async_eth_usd_update.hpp"
#include "drivers/blockchain_client.hpp"
#include "config/configuration.hpp"
void Async_ETH_USD_Currency::request_eth_price(void) {

  if (updating)
    return;

  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now - last_update_time)
                     .count();
  if (elapsed < ETH_PRICE_TIMEOUT)
    return;

  updating = true;

  worker = std::async(std::launch::async, []() {
    return BlockchainClient::get_eth_price_in_usd();
  });
}

void Async_ETH_USD_Currency::update(void) {
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

double Async_ETH_USD_Currency::get_current_eth_price(void) const {
  return this->current_price;
}

bool Async_ETH_USD_Currency::get_status(void) const { return this->updating; }
