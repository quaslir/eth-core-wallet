#include "drivers/balance_client.hpp"
#include "api/http.hpp"
#include "api/json.hpp"
#include "config/configuration.hpp"
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

void BalanceManager::request(const secure_string &addr) {
if(!can_request()) return;
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


void BalanceManager::clear_timer(void) {
  last_update_time =
      std::chrono::steady_clock::now() - std::chrono::milliseconds(TRANSACTION_TIMEOUT);
}

void BalanceManager::clear(void) {
  current_balance = 0.0;
  updating = false;
}
