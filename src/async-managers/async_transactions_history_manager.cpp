#include "async-managers/async_transactions_history_manager.hpp"

void AsyncTransactionsHistoryManager::request_transactions_data(
    const std::string &eth_addr) {
  if (updating)
    return;

  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now - last_update_time)
                     .count();
  if (elapsed < TIMER)
    return;

  updating = true;

  worker = std::async(std::launch::async, [this, eth_addr]() {
    return block_client.get_transaction_history(eth_addr);
  });
}
void AsyncTransactionsHistoryManager::update(void) {
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
bool AsyncTransactionsHistoryManager::get_status(void) const {
  return this->updating;
}
std::vector<TransactionRecord>
AsyncTransactionsHistoryManager::get_transactions_history(void) const {
  return this->current_transactions_history;
}
