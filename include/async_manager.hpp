#pragma once
#include "blockchain_client.hpp"
#include <chrono>
#include <future>
#include <string>
#define TIMER 1000
class AsyncBalanceManager {
private:
  BlockchainClient block_client;
  std::future<std::string> worker;
  bool updating = false;
  std::string current_balance = "0";
std::chrono::steady_clock::time_point last_update_time;
public:

AsyncBalanceManager() : last_update_time(std::chrono::steady_clock::now() - std::chrono::milliseconds(TIMER)) {}
  void request_balance(const std::string &addr);
  void update(void);
  std::string get_balance(void) const;
  bool get_status(void) const;

  void clear_timer(void);
};
