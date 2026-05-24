#include "config/configuration.hpp"
#include "core/secure_bytes_data.hpp"
#include "drivers/manager.hpp"
#include <cstdint>
#include <future>

class GasManager : public Manager {
private:
  std::future<std::pair<double, uint64_t>> worker;
  double current_gas_price = 0;

  double request_gas(void);

public:
  GasManager() : Manager(GWEI_TIMEOUT) {}
  ~GasManager();
  std::function<std::string(void)> form_url;

  void request(const secure_string &eth_addr = "") override;
  void update(void) override;

  double get_current_gas(void) const;
};
