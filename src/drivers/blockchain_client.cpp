#include "drivers/blockchain_client.hpp"
#include "core/secure_bytes_data.hpp"
#include <string>

BlockchainClient::BlockchainClient(void) {
auto form_url_callback = [this](void) -> std::string {
    return form_url();
};

history_manager.form_url = form_url_callback;
price_manager.form_url = form_url_callback;
balance_manager.form_url = form_url_callback;
}


void BlockchainClient::update(void) {
    if(history_manager.get_status()) history_manager.update();
    else history_manager.make_request_transaction_history(std::string{get_current_eth_addr()});
    if(price_manager.get_status()) price_manager.update();
    else price_manager.get_eth_price_in_usd();
    if(balance_manager.get_status()) balance_manager.update();
    else balance_manager.make_request(get_current_eth_addr());
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


double BlockchainClient::get_balance(void) const {
    return balance_manager.get_balance();
}
std::vector<TransactionRecord> BlockchainClient::get_transaction_history(void) const {
    return history_manager.get_transactions_history();
}
double BlockchainClient::get_eth_price(void) const {
    return price_manager.get_current_eth_price();
}


void BlockchainClient::update_history_manager(void) {
    if(!get_current_eth_addr) return;
    secure_string eth_addr = get_current_eth_addr();
    if(eth_addr.empty()) return;
    history_manager.request_transactions_data(std::string{eth_addr});
    history_manager.update();
}
void BlockchainClient::update_price_manager(void) {
    price_manager.request_eth_price();
    price_manager.update();
}
void BlockchainClient::update_balance_manager(void) {
    if(!get_current_eth_addr) return;
    secure_string eth_addr = get_current_eth_addr();
    if(eth_addr.empty()) return;
    balance_manager.request_balance( eth_addr );
    balance_manager.update();
}
