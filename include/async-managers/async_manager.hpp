#pragma once
#include "drivers/blockchain_client.hpp"
#include <chrono>
#include <future>
#include <string>
#define TIMER 1000
class AsyncBalanceManager {
private:
  const BlockchainClient &block_client;
  std::future<double> worker;
  bool updating = false;
  double current_balance = 0.0;
  std::chrono::steady_clock::time_point last_update_time;

public:
  AsyncBalanceManager(const BlockchainClient &client)
      : block_client(client),
        last_update_time(std::chrono::steady_clock::now() -
                         std::chrono::milliseconds(TIMER)) {}
  void request_balance(const std::string &addr);
  void update(void);
  double get_balance(void) const;
  bool get_status(void) const;
  void clear(void);
  void clear_timer(void);
};
