#pragma once
#include "drivers/blockchain_client.hpp"
#include <chrono>
#include <future>
#include <vector>
#define TIMER 1000

class AsyncTransactionsHistoryManager {
private:
  std::future<std::vector<TransactionRecord>> worker;
  bool updating = false;
  BlockchainClient &block_client;
  std::chrono::steady_clock::time_point last_update_time;
  std::vector<TransactionRecord> current_transactions_history;
  bool error = false;
public:
  AsyncTransactionsHistoryManager(BlockchainClient &client);

  void request_transactions_data(const std::string &eth_addr);
  void update(void);
  bool get_status(void) const;
  bool get_error(void) const;
  std::vector<TransactionRecord> get_transactions_history(void) const;
};
