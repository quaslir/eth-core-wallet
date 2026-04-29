#include "config/configuration.hpp"
#include <chrono>
#include <future>

class Async_ETH_USD_Currency {
private:
  double current_price = 0.0; // for 1 ETH, data is taken from coingecko
  std::future<double> worker;
  bool updating = false;
  std::chrono::steady_clock::time_point last_update_time;

public:
  Async_ETH_USD_Currency()
      : last_update_time(std::chrono::steady_clock::now() -
                         std::chrono::milliseconds(ETH_PRICE_TIMEOUT)) {}

  bool get_status(void) const;
  void request_eth_price(void);
  void update(void);
  double get_current_eth_price(void) const;
};
