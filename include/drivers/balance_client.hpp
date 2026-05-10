#pragma once
#include "config/configuration.hpp"
#include "core/secure_bytes_data.hpp"
#include "drivers/manager.hpp"
#include <functional>
#include <future>
#include <string>
class BalanceManager : public Manager {
private:
  std::future<double> worker;
  double current_balance = 0.0;
  double make_request(const secure_string &eth_addr) const;

public:
  std::function<std::string(void)> form_url;
  BalanceManager() : Manager(BALANCE_TIMEOUT) {}

  void request(const secure_string &eth_addr) override;
  void update(void) override;
  double get_balance(void) const;
  void clear(void);
  void clear_timer(void);
};
