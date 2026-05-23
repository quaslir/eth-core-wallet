
#include "drivers/tx_status_client.hpp"
#include "api/http.hpp"
#include "api/json.hpp"
#include <exception>
#include <iostream>
TxStatusManager::~TxStatusManager() {
  if (worker.valid()) {
    worker.wait();
  }
}

void TxStatusManager::request(const secure_string &) {

  if (updating || current_tx_hash.empty())
    return;
  updating = true;

  worker = std::async(std::launch::async, [this]() { return make_request(); });
}
void TxStatusManager::update(void) {
  if (updating && !current_tx_hash.empty() && worker.valid()) {
    auto status = worker.wait_for(std::chrono::milliseconds(0));

    if (status == std::future_status::ready) {
      try {
        if (!error)
          current_tx_status = worker.get();
      } catch (const std::exception &err) {
      }

      updating = false;
      last_update_time = std::chrono::steady_clock::now();
    }
  }
}
std::pair<TxStatus, bool> TxStatusManager::get_current_tx_status(void) const {
  return current_tx_status;
}

void TxStatusManager::set_tx_hash(const std::string &tx_hash) {
  current_tx_hash = tx_hash;
}

std::pair<TxStatus, bool> TxStatusManager::make_request(void) {
  if (current_tx_hash.empty())
    return {TxStatus::ERROR, true};
  try {
    json req = {{"jsonrpc", "2.0"},
                {"id", 1},
                {"method", "eth_getTransactionReceipt"},
                {"params", json::array({current_tx_hash})}

    };
    std::string data = req.dump();
    std::string buffer = http::post_request(form_url(), data);
    json res = json::parse(buffer);
    std::cerr << buffer << std::endl;
    if (res.contains("error")) {
      return {TxStatus::ERROR, false};
    }

    if (res["result"].is_null()) {
      return {TxStatus::PENDING, false};
    }

    std::string status_hex = res["result"].at("status").get<std::string>();
    if (status_hex == "0x1") {
      return {TxStatus::SUCCESS, true};
    } else
      return {TxStatus::FAILED, false};
  } catch (const std::exception &err) {
    return {TxStatus::ERROR, true};
  }
}
