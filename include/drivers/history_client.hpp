#pragma once
#include "api/json.hpp"
#include "config/configuration.hpp"
#include "core/secure_bytes_data.hpp"
#include "drivers/manager.hpp"
#include <chrono>
#include <cstdint>
#include <functional>
#include <future>
#include <string>
#include <vector>

struct TransactionRecord {
  std::string hash;
  std::string block_num;
  double value;
  std::string asset;
  std::string from;
  std::string to;
  std::string timestamp;
  bool incoming;
};
class HistoryManager : public Manager {
private:
  std::future<std::pair<std::vector<TransactionRecord>, uint64_t>> worker;
  std::vector<TransactionRecord> cached_history{};
  std::string last_known_block{"0x0"};

#if defined(__cpp_lib_atomic_shared_ptr) &&                                    \
    __cpp_lib_atomic_shared_ptr >= 201711L
  std::atomic<std::shared_ptr<std::vector<TransactionRecord>>> atomic_history{
      std::make_shared<std::vector<TransactionRecord>>()};
#else
  std::shared_ptr<std::vector<TransactionRecord>> atomic_history{
      std::make_shared<std::vector<TransactionRecord>>()};
#endif

  std::vector<TransactionRecord> parse_transactions(const json &j,
                                                    bool incoming = true) const;

  std::vector<TransactionRecord> make_request(const std::string &eth_addr);

public:
  HistoryManager() : Manager(TRANSACTION_TIMEOUT) {}
  ~HistoryManager();
  std::function<std::string(void)> form_url;

  void request(const secure_string &eth_addr) override;
  void update(void) override;
  std::shared_ptr<std::vector<TransactionRecord>>
  get_transactions_history(void) const;
  void clear_history(void);
};
