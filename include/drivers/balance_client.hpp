#pragma once
#include "core/secure_bytes_data.hpp"

#include <chrono>
#include <functional>
#include <future>
#include <string>
#define TIMER 1000
class BalanceManager {
private:
  std::future<double> worker;
  bool updating = false;
  double current_balance = 0.0;
  std::chrono::steady_clock::time_point last_update_time;



public:
  std::function<std::string(void)> form_url;
  BalanceManager()
      : last_update_time(std::chrono::steady_clock::now() -
                         std::chrono::milliseconds(TIMER)) {}
  void request_balance(const secure_string &eth_addr);
  void update(void);
  double get_balance(void) const;
  bool get_status(void) const;
  void clear(void);
  void clear_timer(void);
    double make_request(const secure_string &eth_addr) const;
};
