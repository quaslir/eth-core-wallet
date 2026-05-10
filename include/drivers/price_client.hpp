#include "config/configuration.hpp"
#include "core/secure_bytes_data.hpp"
#include "drivers/manager.hpp"
#include <chrono>
#include <cstdint>
#include <future>

class PriceManager : public Manager {
private:
  double current_price = 0.0;
  std::future<double> worker;
  double request_eth_price(void) const;

public:
  std::function<std::string(void)> form_url;
  PriceManager() : Manager(ETH_PRICE_TIMEOUT) {}

  void request(const secure_string &) override;
  void update(void) override;
  double get_current_eth_price(void) const;
};
