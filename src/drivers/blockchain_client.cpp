#include "drivers/blockchain_client.hpp"
#include "core/secure_bytes_data.hpp"
#include "drivers/balance_client.hpp"
#include <string>
#include <iostream>
BlockchainClient::BlockchainClient(void) {
  auto form_url_callback = [this](void) -> std::string { return form_url(); };

  history_manager.form_url = form_url_callback;
  price_manager.form_url = form_url_callback;
  balance_manager.form_url = form_url_callback;
  gas_manager.form_url = form_url_callback;
}

void BlockchainClient::update(void) {
  if (history_manager.get_status())
    history_manager.update();
  else
    history_manager.request(get_current_eth_addr());
  if (price_manager.get_status())
    price_manager.update();
  else
    price_manager.request(secure_string{});
  if (balance_manager.get_status())
    balance_manager.update();
  else
    balance_manager.request(get_current_eth_addr());
}

void BlockchainClient::change_network(
    const networks::NetworkConfig &new_network) {
  active_network = new_network;
}

std::string BlockchainClient::form_url(void) const {
  return "https://" + active_network.rpc_prefix + ".g.alchemy.com/v2/" +
         "MkveNSvN4rHOvLoZK8dE3";
}

std::string BlockchainClient::get_active_network_name(void) const {
  return active_network.name;
}

assets_data BlockchainClient::get_balance(void) const {

 return balance_manager.get_balance();

}
std::pair<std::vector<TransactionRecord>, bool>
BlockchainClient::get_transaction_history(void) const {
  return {history_manager.get_transactions_history(),
          history_manager.get_error()};
}
std::pair<double, bool> BlockchainClient::get_eth_price(void) const {
  return {price_manager.get_current_eth_price(), price_manager.get_error()};
}

std::pair<double, bool> BlockchainClient::get_current_gas(void) const {
  return {gas_manager.get_current_gas(), gas_manager.get_error()};
}

void BlockchainClient::update_history_manager(bool force) {
  if (!get_current_eth_addr)
    return;
  secure_string eth_addr = get_current_eth_addr();
  if (eth_addr.empty())
    return;
  if (force)
    history_manager.request(eth_addr);
  else
    history_manager.force_request(eth_addr);
  history_manager.update();
}
void BlockchainClient::update_price_manager(bool force) {
  if (force)
    price_manager.force_request();
  else
    price_manager.request(secure_string{});

  price_manager.update();
}
void BlockchainClient::update_balance_manager(bool force) {
  if (!get_current_eth_addr)
    return;
  secure_string eth_addr = get_current_eth_addr();
  if (eth_addr.empty())
    return;

  if (force)
    balance_manager.force_request(eth_addr);
  else
    balance_manager.request(eth_addr);

  balance_manager.update();
}

void BlockchainClient::update_gas_manager(bool force) {
  if (force)
    gas_manager.force_request();
  else
    gas_manager.request();

  gas_manager.update();
}
