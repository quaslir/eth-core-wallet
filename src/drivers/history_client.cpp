#include "drivers/history_client.hpp"
#include "api/http.hpp"
#include "api/json.hpp"
#include "core/secure_bytes_data.hpp"
#include "utils/tech_utils.hpp"
#include <algorithm>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <utility>
#include <vector>
std::vector<TransactionRecord> HistoryManager::parse_transactions(
    const json &j, const std::string &target_addr, bool is_erc20) const {

  std::vector<TransactionRecord> history;

  if (!j.contains("status") || j["status"].get<std::string>() != "1") {
    return history;
  }
  std::string lower_target = tech_utils::tolower(target_addr);

  for (const auto &item : j["result"]) {
    TransactionRecord tx;
    tx.hash = item.value("hash", "");
    tx.from = item.value("from", "");
    tx.to = item.value("to", "");
    tx.incoming = (tech_utils::tolower(tx.to) == lower_target);
    std::string unix_time = item.value("timeStamp", "0");
    tx.unix_time = std::stoull(unix_time);
    tx.timestamp = tech_utils::format_unix_timestamp(unix_time);
    tx.block_num = item.value("blockNumber", "0");
    std::string raw_value = item.value("value", "0");

    double val_double = 0.0;
    try {
      val_double = std::stod(raw_value);

    } catch (...) {
      val_double = 0.0;
    }

    if (is_erc20) {
      tx.asset = item.value("tokenSymbol", "UNKNOWN");
      int decimals = 18;
      if (item.contains("tokenDecimal") &&
          !item["tokenDecimal"].get<std::string>().empty()) {
        try {
          decimals = std::stoi(item["tokenDecimal"].get<std::string>());
        } catch (...) {
        }
      }

      tx.value = val_double / std::pow(10, decimals);
    } else {
      tx.asset = "ETH";
      tx.value = val_double / 1e18;
    }

    if (tx.value > 0.0 || !is_erc20) {

      history.push_back(tx);
    }
  }
  return history;
}

std::vector<TransactionRecord>
HistoryManager::make_request(const std::string &eth_addr) {
  cached_history.clear();
  std::string base_url{};

  try {
    std::string res_native = http::get_request(form_native_url(eth_addr));
    std::string res_erc20 = http::get_request(form_erc20_url(eth_addr));

    auto history_native =
        parse_transactions(json::parse(res_native), eth_addr, false);
    auto history_erc20 =
        parse_transactions(json::parse(res_erc20), eth_addr, true);

    cached_history.insert(cached_history.end(), history_native.begin(),
                          history_native.end());
    cached_history.insert(cached_history.end(), history_erc20.begin(),
                          history_erc20.end());
  } catch (const std::exception &err) {
    return cached_history;
  }

  std::sort(
      cached_history.begin(), cached_history.end(),
      [](const auto &a, const auto &b) { return a.unix_time > b.unix_time; });

  return cached_history;
}

void HistoryManager::request(const secure_string &eth_addr) {
  if (!can_request())
    return;

  updating = true;
  uint64_t gen = get_generation();
  worker = std::async(std::launch::async, [this, eth_addr, gen]() {
    return std::make_pair(make_request(std::string{eth_addr}), gen);
  });
}
void HistoryManager::update(void) {
  if (updating && worker.valid()) {
    auto status = worker.wait_for(std::chrono::milliseconds(0));

    if (status == std::future_status::ready) {
      try {
        auto [trans_history, gen] = worker.get();
        if (gen == get_generation()) {
          auto ptr = std::make_shared<std::vector<TransactionRecord>>(
              std::move(trans_history));
#if defined(__cpp_lib_atomic_shared_ptr) &&                                    \
    __cpp_lib_atomic_shared_ptr >= 201711L
          atomic_history.store(ptr);
#else
          std::atomic_store(&atomic_history, ptr);
#endif
        }
      } catch (const std::exception &err) {
      }

      updating = false;
      last_update_time = std::chrono::steady_clock::now();
    }
  }
}

std::shared_ptr<std::vector<TransactionRecord>>
HistoryManager::get_transactions_history(void) const {
#if defined(__cpp_lib_atomic_shared_ptr) &&                                    \
    __cpp_lib_atomic_shared_ptr >= 201711L
  return atomic_history.load();
#else
  return std::atomic_load(&atomic_history);
#endif
}

HistoryManager::~HistoryManager() {
  if (worker.valid()) {
    worker.wait();
  }
}

void HistoryManager::clear_history(void) {
  cached_history.clear();
  last_known_block = "0x0";
}
