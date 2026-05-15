#include "drivers/blockchain_client.hpp"
#include "config/configuration.hpp"
#include "core/secure_bytes_data.hpp"
#include "drivers/balance_client.hpp"
#include <chrono>
#include <string>
#include <iostream>
BlockchainClient::BlockchainClient(void) : last_update_time(std::chrono::steady_clock::now() - std::chrono::milliseconds(FULL_UPDATE_TIMEOUT)){
  auto form_url_callback = [this](void) -> std::string { return form_url(); };

  history_manager.form_url = form_url_callback;
  balance_manager.form_url = form_url_callback;
  gas_manager.form_url = form_url_callback;
}

void BlockchainClient::update(void) {
    balance_manager.update();
    history_manager.update();
    gas_manager.update();

    auto now = std::chrono::steady_clock::now();
    if((now - last_update_time >= std::chrono::milliseconds(FULL_UPDATE_TIMEOUT))) {
 bool upd_balance =  update_balance_manager(true);
 bool upd_history =  update_history_manager(true);
  bool upd_gas = update_gas_manager(true);
  if(upd_balance && upd_history && upd_gas) {

  last_update_time = std::chrono::steady_clock::now();

  }
    }
}

void BlockchainClient::change_network(
    const networks::NetworkConfig &new_network) {
  active_network = new_network;

  push_activity("🔗", "Switched to " + new_network.name);
}

std::string BlockchainClient::form_url(void) const {
  return "https://" + active_network.rpc_prefix + ".g.alchemy.com/v2/" +
         "MkveNSvN4rHOvLoZK8dE3";
}

std::string BlockchainClient::get_active_network_name(void) const {
  return active_network.name;
}

 std::shared_ptr<assets_data> BlockchainClient::get_balance(void) const {

  return balance_manager.get_balance();
}
std::pair<std::vector<TransactionRecord>, bool>
BlockchainClient::get_transaction_history(void) const {
  return {history_manager.get_transactions_history(),
          history_manager.get_error()};
}

std::pair<double, bool> BlockchainClient::get_current_gas(void) const {
  return {gas_manager.get_current_gas(), gas_manager.get_error()};
}

bool BlockchainClient::update_history_manager(bool force) {
  if (!get_current_eth_addr)
    return false;
  secure_string eth_addr = get_current_eth_addr();
  if (eth_addr.empty())
    return false;
  if (force)
       history_manager.force_request(eth_addr);

  else
          history_manager.request(eth_addr);

  return true;
}

bool BlockchainClient::update_balance_manager(bool force) {
  if (!get_current_eth_addr)
    return false;
  secure_string eth_addr = get_current_eth_addr();
  if (eth_addr.empty())
    return false;

  if (force)
    balance_manager.force_request(eth_addr);
  else
    balance_manager.request(eth_addr);

  return true;
}

bool BlockchainClient::update_gas_manager(bool force) {
    if (!get_current_eth_addr) {
            return false;
    }
        if (get_current_eth_addr().empty()) {
            return false;
        }
  if (force)
    gas_manager.force_request();
  else
    gas_manager.request();

  return true;
}

float BlockchainClient::get_next_refresh(void) const {

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now - last_update_time)
                       .count();
    float progress =
        static_cast<float>(elapsed) / static_cast<float>(FULL_UPDATE_TIMEOUT);

    return std::min(progress, 1.0f);
}

void BlockchainClient::push_activity(const std::string& icon, const std::string& msg) {
    activity_log.push_front({icon, msg, std::chrono::system_clock::now()});

    if(activity_log.size() > 5) {
        activity_log.pop_back();
    }
}

const std::deque<ActivityEvent>&BlockchainClient:: get_activity(void) const {
    return activity_log;
}
