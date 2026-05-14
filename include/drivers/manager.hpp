#pragma once

#include "core/secure_bytes_data.hpp"
#include <atomic>
#include <chrono>
#include <cstdint>
class Manager {
protected:
  std::atomic<bool> updating = false;
  const std::chrono::milliseconds timer_interval;
  std::chrono::steady_clock::time_point last_update_time;
  bool error = false;

public:
  Manager(uint32_t ms_interval)
      : timer_interval(ms_interval),
        last_update_time(std::chrono::steady_clock::now() -
                         std::chrono::milliseconds(ms_interval)) {}
  virtual ~Manager() = default;
  virtual void request(const secure_string &eth_addr = "") = 0;
  virtual void update(void) = 0;

  void force_request(const secure_string &eth_addr = "");
  bool get_status(void) const;
  bool get_error(void) const;
  bool can_request(void) const;
};
