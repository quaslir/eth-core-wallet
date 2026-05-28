#include "drivers/history_client.hpp"
#include "api/http.hpp"
#include "api/json.hpp"
#include "core/secure_bytes_data.hpp"
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

std::vector<TransactionRecord>
HistoryManager::parse_transactions(const json &j, bool incoming) const {
  try {
    std::vector<TransactionRecord> history;

    if (!j.contains("result") || j["result"].is_null()) {
      return std::vector<TransactionRecord>();
    }

    if (!j["result"].contains("transfers") ||
        j["result"]["transfers"].is_null()) {
      return std::vector<TransactionRecord>();
      ;
    }
    for (const auto &item : j["result"]["transfers"]) {
      TransactionRecord tx;
      tx.hash = item["hash"].get<std::string>();
      tx.value = item.is_null() ? 0.0 : item["value"].get<double>();
      tx.asset = item["asset"].get<std::string>();
      tx.incoming = incoming;
      if (item.contains("from")) {
        tx.from = item["from"].get<std::string>();
      }
      if (item.contains("to")) {
        tx.to = item["to"].get<std::string>();
      }

      if (item.contains("metadata") &&
          !item["metadata"]["blockTimestamp"].is_null()) {
        tx.timestamp = item["metadata"]["blockTimestamp"].get<std::string>();
      }

      if(item.contains("blockNum")) {
          tx.block_num = item["blockNum"].get<std::string>();
      }

      history.push_back(tx);
    }
    return history;
  } catch (const std::exception &err) {
    return std::vector<TransactionRecord>();
  }
}

std::vector<TransactionRecord>
HistoryManager::make_request(const std::string &eth_addr) {

  json request_body_1 = transactions_history::form_receives(eth_addr, last_known_block);
  json request_body_2 = transactions_history::form_sends(eth_addr, last_known_block);

  auto make_request_and_parse_buffer =
      [this](const json &j) -> std::pair<json, bool> {
    try {
      std::string data = j.dump();
      std::string buffer = http::post_request(form_url(), data);
      json res = json::parse(buffer);
      return {res, true};
    } catch (const std::exception &err) {
      return {json::object(), false};
    }
  };

  auto future_in = std::async(std::launch::async, make_request_and_parse_buffer,
                              std::cref(request_body_1));
  auto future_out =
      std::async(std::launch::async, make_request_and_parse_buffer,
                 std::cref(request_body_2));

  auto [object_in, status_in] = future_in.get();
  auto [object_out, status_out] = future_out.get();

  auto history_in = parse_transactions(object_in);
  auto history_out = parse_transactions(object_out, false);

  cached_history.insert(cached_history.end(), history_in.begin(), history_in.end());
  cached_history.insert(cached_history.end(), history_out.begin(), history_out.end());

  std::sort(
      cached_history.begin(), cached_history.end(),
      [](const auto &a, const auto &b) { return b.timestamp < a.timestamp; });

  if(!cached_history.empty()) {

  last_known_block = cached_history.front().block_num;
  }
  return cached_history;
}

void HistoryManager::request(const secure_string &eth_addr) {
  if (!can_request())
    return;

  updating = true;
    uint64_t gen = get_generation();
  worker = std::async(std::launch::async, [this, eth_addr, gen]() {
    return  std::make_pair(make_request(std::string{eth_addr}), gen);
  });
}
void HistoryManager::update(void) {
  if (updating && worker.valid()) {
    auto status = worker.wait_for(std::chrono::milliseconds(0));

    if (status == std::future_status::ready) {
      try {
          auto [trans_history, gen] = worker.get();
          if(gen == get_generation()) {
              auto ptr = std::make_shared<std::vector<TransactionRecord>>(std::move(trans_history));
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
