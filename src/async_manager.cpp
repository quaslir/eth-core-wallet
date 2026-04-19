#include "async_manager.hpp"
#include <chrono>
#include <exception>


void AsyncBalanceManager::request_balance(const std::string &addr) {
  if (updating)
    return;

  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update_time).count();
  if(elapsed < TIMER) return;


  updating = true;

  worker = std::async(std::launch::async, [this, addr]() {
    return block_client.get_balance(addr);
  });
}

void AsyncBalanceManager::update(void) {
  if (updating && worker.valid()) {
    auto status = worker.wait_for(std::chrono::milliseconds(0));

    if (status == std::future_status::ready) {
      try {
        current_balance = worker.get();
      } catch (const std::exception &err) {
        current_balance = "0";
      }
      updating = false;
      last_update_time = std::chrono::steady_clock::now();
    }
  }
}
std::string AsyncBalanceManager::get_balance(void) const {
  return this->current_balance;
}
bool AsyncBalanceManager::get_status(void) const {
    return updating;
}
