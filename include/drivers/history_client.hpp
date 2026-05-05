#pragma once
#include <chrono>
#include <functional>
#include <future>
#include <string>
#include <vector>
#include "api/json.hpp"
#define TIMER 1000

struct TransactionRecord {
  std::string hash;
  double value;
  std::string asset;
  std::string from;
  std::string to;
  std::string timestamp;
  bool incoming;
};
class HistoryManager {
private:
  std::future<std::vector<TransactionRecord>> worker;
  bool updating = false;
  std::chrono::steady_clock::time_point last_update_time;
  std::vector<TransactionRecord> current_transactions_history;
  bool error = false;


  std::vector<TransactionRecord> parse_transactions(const json &j, bool incoming = true) const;
public:
std::function<std::string(void)> form_url;
  HistoryManager();

  void request_transactions_data(const std::string &eth_addr);
  void update(void);
  bool get_status(void) const;
  bool get_error(void) const;
  std::vector<TransactionRecord> get_transactions_history(void) const;
    std::vector<TransactionRecord> make_request_transaction_history(const std::string &eth_addr) const;
};
