#include "drivers/balance_client.hpp"
#include "api/http.hpp"
#include "api/json.hpp"
#include "core/secure_bytes_data.hpp"
#include "core/uint256.hpp"
#include "utils/tech_utils.hpp"
#include <chrono>
#include <exception>
double BalanceManager::make_request(const secure_string &eth_addr) const {

  try {
    GetBalanceMethod alchm("2.0", "eth_getBalance",
                           {std::string{eth_addr}, "latest"}, 1);

    std::string data = alchm.to_string();

    std::string buffer = http::post_request(form_url(), data);
    if (buffer.empty())
      return 0.0;

    alchm.parse(buffer);

    Uint256 uint256_t(alchm.get_result(), 1);

    std::string res = uint256_t.from_wei_to_eth();

    double value;

    if (!tech_utils::to_double(res, value))
      return 0.0;

    return value;

  } catch (const std::exception &err) { // handle in the future
                                        // handle
    return 0.0;
  }
}

void BalanceManager::request_balance(const secure_string &addr) {
  if (updating)
    return;

  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now - last_update_time)
                     .count();
  if (elapsed < TIMER)
    return;

  updating = true;

  worker = std::async(std::launch::async,
                      [this, addr]() { return make_request(addr); });
}

void BalanceManager::update(void) {
  if (updating && worker.valid()) {
    auto status = worker.wait_for(std::chrono::milliseconds(0));

    if (status == std::future_status::ready) {
      try {
        current_balance = worker.get();
      } catch (const std::exception &err) {
        current_balance = 0.0;
      }
      updating = false;
      last_update_time = std::chrono::steady_clock::now();
    }
  }
}
double BalanceManager::get_balance(void) const { return this->current_balance; }
bool BalanceManager::get_status(void) const { return updating; }

void BalanceManager::clear_timer(void) {
  last_update_time =
      std::chrono::steady_clock::now() - std::chrono::milliseconds(TIMER);
}

void BalanceManager::clear(void) {
  current_balance = 0.0;
  updating = false;
}
