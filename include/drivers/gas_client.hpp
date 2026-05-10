#include "config/configuration.hpp"
#include "core/secure_bytes_data.hpp"
#include "drivers/manager.hpp"
#include <future>

class GasManager : public Manager {
private:
  std::future<double> worker;
  double current_gas_price = 0;

  double request_gas(void) const;

public:
  GasManager() : Manager(GWEI_TIMEOUT) {}

  std::function<std::string(void)> form_url;

  void request(const secure_string &eth_addr = "") override;
  void update(void) override;

  double get_current_gas(void) const;
};
