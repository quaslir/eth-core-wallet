#include "drivers/history_client.hpp"
#include "api/http.hpp"
#include "core/secure_bytes_data.hpp"

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

      history.push_back(tx);
    }
    return history;
  } catch (const std::exception &err) {
    return std::vector<TransactionRecord>();
  }
}

std::vector<TransactionRecord>
HistoryManager::make_request(const std::string &eth_addr) const {

  json request_body_1 = transactions_history::form_receives(eth_addr);
  json request_body_2 = transactions_history::form_sends(eth_addr);

  auto make_request_and_parse_buffer = [this](const json &j) {
    try {
      std::string data = j.dump();
      std::string buffer = http::post_request(form_url(), data);
      json res = json::parse(buffer);
      return res;
    } catch (const std::exception &err) {
      return json::object();
    }
  };

  json object_in = make_request_and_parse_buffer(request_body_1);
  json object_out = make_request_and_parse_buffer(request_body_2);

  auto history_in = parse_transactions(object_in);
  auto history_out = parse_transactions(object_out, false);

  std::vector<TransactionRecord> full_history = std::move(history_in);
  full_history.insert(full_history.end(), history_out.begin(),
                      history_out.end());

  std::sort(
      full_history.begin(), full_history.end(),
      [](const auto &a, const auto &b) { return b.timestamp < a.timestamp; });
  return full_history;
}

void HistoryManager::request(const secure_string &eth_addr) {
  if (!can_request())
    return;

  updating = true;

  worker = std::async(std::launch::async, [this, eth_addr]() {
    return make_request(std::string{eth_addr});
  });
}
void HistoryManager::update(void) {
  if (updating && worker.valid()) {
    auto status = worker.wait_for(std::chrono::milliseconds(0));

    if (status == std::future_status::ready) {
      try {
        current_transactions_history = worker.get();
      } catch (const std::exception &err) {
        current_transactions_history = {};
      }
      updating = false;
      last_update_time = std::chrono::steady_clock::now();
    }
  }
}

std::vector<TransactionRecord>
HistoryManager::get_transactions_history(void) const {
  return this->current_transactions_history;
}
