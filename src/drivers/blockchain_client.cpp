#include "drivers/blockchain_client.hpp"
#include "config/configuration.hpp"
#include "core/assets.hpp"
#include "core/secure_bytes_data.hpp"
#include "core/uint256.hpp"
#include "drivers/balance_client.hpp"
#include "fmt/core.h"
#include "fmt/format.h"
#include "utils/tech_utils.hpp"
#include <chrono>
#include <cstdint>
#include <exception>
#include <string>
BlockchainClient::BlockchainClient(void)
    : last_update_time(std::chrono::steady_clock::now() -
                       std::chrono::milliseconds(
                           Configuration::get_instance().FULL_UPDATE_TIMEOUT)) {
  auto form_url_callback = [this](void) -> std::string { return form_url(); };

  balance_manager.form_url = form_url_callback;
  gas_manager.form_url = form_url_callback;
  transaction_manager.form_url = form_url_callback;
  tx_status_manager.form_url = form_url_callback;
  tokens_metadata_manager.form_url = form_url_callback;

  history_manager.form_native_url =
      [this](const std::string &eth_addr) -> std::string {
    return fmt::format(
        "https://api.etherscan.io/v2/"
        "api?chainid={}&module=account&action=txlist&address={}&startblock=0&"
        "endblock=99999999&page=1&offset=50&sort=desc&apikey={}",
        active_network.chain_id, eth_addr,
        Configuration::get_instance().get_etherscan_api());
  };

  history_manager.form_erc20_url =
      [this](const std::string &eth_addr) -> std::string {
    return fmt::format(
        "https://api.etherscan.io/v2/"
        "api?chainid={}&module=account&action=tokentx&address={}&startblock=0&"
        "endblock=99999999&page=1&offset=50&sort=desc&apikey={}",
        active_network.chain_id, eth_addr,
        Configuration::get_instance().get_etherscan_api());
  };

  balance_manager.set_current_chain_id_callback(
      [this]() -> uint64_t { return active_network.chain_id; });
  balance_manager.set_current_assets_callback(
      [this](uint64_t chain_id) -> assets_data {
        return get_current_assets(chain_id);
      });

  tokens_metadata_manager.get_chain_id = [this]() -> uint64_t {
    return active_network.chain_id;
  };
}

void BlockchainClient::update(void) {
  balance_manager.update();
  history_manager.update();
  gas_manager.update();
  auto now = std::chrono::steady_clock::now();
  if ((now - last_update_time >=
       std::chrono::milliseconds(
           Configuration::get_instance().FULL_UPDATE_TIMEOUT))) {
    bool upd_balance = update_balance_manager(true);
    bool upd_history = update_history_manager(true);
    bool upd_gas = update_gas_manager(true);
    update_current_tx_status();
    if (upd_balance && upd_history && upd_gas) {

      last_update_time = std::chrono::steady_clock::now();
    }
  }
}

void BlockchainClient::change_network(
    const networks::NetworkConfig &new_network) {
  active_network = new_network;
  balance_manager.clear();
  gas_manager.clear();
  history_manager.clear();

  last_update_time = std::chrono::steady_clock::now() -
                     std::chrono::milliseconds(
                         Configuration::get_instance().FULL_UPDATE_TIMEOUT);

  update();
  push_activity("🔗", "Switched to " + new_network.name);
}

std::string BlockchainClient::form_url(void) const {
  return "https://" + active_network.rpc_prefix + ".g.alchemy.com/v2/" +
         Configuration::get_instance().get_alchemy_api();
}

std::string BlockchainClient::get_active_network_name(void) const {
  return active_network.name;
}

std::shared_ptr<assets_data> BlockchainClient::get_balance(void) const {

  return balance_manager.get_balance();
}
std::pair<std::shared_ptr<std::vector<TransactionRecord>>, bool>
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

void BlockchainClient::clear_history(void) { history_manager.clear_history(); }

std::pair<TxStatus, bool> BlockchainClient::get_current_tx_status(void) const {
  return tx_status_manager.get_current_tx_status();
}
void BlockchainClient::update_current_tx_status(void) {
  if (!get_current_eth_addr) {
    return;
  }
  if (tx_status_manager.get_status()) {
    tx_status_manager.update();
  } else
    tx_status_manager.request(secure_string{});
}

float BlockchainClient::get_next_refresh(void) const {

  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now - last_update_time)
                     .count();
  float progress =
      static_cast<float>(elapsed) /
      static_cast<float>(Configuration::get_instance().FULL_UPDATE_TIMEOUT);

  return std::min(progress, 1.0f);
}

void BlockchainClient::push_activity(const std::string &icon,
                                     const std::string &msg) {
  activity_log.push_front({icon, msg, std::chrono::system_clock::now()});

  if (activity_log.size() > 5) {
    activity_log.pop_back();
  }
}

const std::deque<ActivityEvent> &BlockchainClient::get_activity(void) const {
  return activity_log;
}

std::pair<std::string, bool> BlockchainClient::send_raw_transaction(
    const secure_string &to_addr, const bytes_data &private_key,
    const Asset &asset, const std::string &value, double target_gas_gwei,
    uint64_t gas_limit) {
  RawTx raw_tx;
  auto nonce =
      transaction_manager.get_nonce(get_current_eth_addr(), form_url());
  if (!nonce) {
    return {"Unable to get nonce", false};
  }

  Uint256 val = Uint256::from_decimal_string(value, asset.decimals);
  raw_tx.private_key = private_key;
  raw_tx.nonce = *nonce;
  raw_tx.v = active_network.chain_id;

  uint64_t gas_in_wei = static_cast<uint64_t>(target_gas_gwei * 1e9);
  raw_tx.gas_price = gas_in_wei;
  if (asset.is_native) {
    raw_tx.value = val;
    raw_tx.to = tech_utils::from_hex_to_bytes(std::string{to_addr});
  } else {
    raw_tx.value = Uint256("0", false);
    raw_tx.to = tech_utils::from_hex_to_bytes(asset.contract_address);
    raw_tx.data = transaction_manager.make_transfer_token_data(
        tech_utils::from_hex_to_bytes(std::string{to_addr}), val);
  }

  if (gas_limit == 0) {
    auto estimated_gas =
        transaction_manager.estimate_gas(raw_tx, get_current_eth_addr());
    if (!estimated_gas)
      return {"Unable to estimate gas", false};

    raw_tx.gas_limit = (*estimated_gas * 125) / 100;
  } else
    raw_tx.gas_limit = gas_limit;

  auto rs = transaction_manager.send(raw_tx);
  auto [hash, error] = rs.get();
  if (hash.empty() || error) {
    return {hash, false};
  }
  tx_status_manager.set_tx_hash(hash);
  std::string short_addr{to_addr.size() >= 10
                             ? to_addr.substr(0, 6) + "..." +
                                   to_addr.substr(to_addr.size() - 4)
                             : to_addr};
  push_activity("Sent",
                fmt::format("{} {} to {}", value, asset.symbol, short_addr));
  return {"", true};
}

std::string
BlockchainClient::form_and_send_tx_from_dapp(json params,
                                             const bytes_data &private_key) {
  try {
    RawTx raw_tx;
    raw_tx.private_key = private_key;
    std::string hex_str = params.value("to", "");
    if (!hex_str.empty()) {
      raw_tx.to = tech_utils::from_hex_to_bytes(hex_str);
    } else
      raw_tx.to = bytes_data();

    raw_tx.value = Uint256(params.value("value", "0x0"), true);
    std::string data_hex = params.value("data", "");

    if (!data_hex.empty()) {
      raw_tx.data = tech_utils::from_hex_to_bytes(data_hex);
    } else
      raw_tx.data = bytes_data();

    if (params.contains("nonce")) {
      std::string nonce_hex = params.at("nonce").get<std::string>();
      raw_tx.nonce = std::stoull(nonce_hex, nullptr, 16);
    } else {
      auto nonce =
          transaction_manager.get_nonce(get_current_eth_addr(), form_url());
      raw_tx.nonce = *nonce;
    }

    if (params.contains("gas")) {
      std::string gas_hex = params.at("gas").get<std::string>();
      raw_tx.gas_limit = std::stoull(gas_hex, nullptr, 16);
    } else {
      auto estimated_gas =
          transaction_manager.estimate_gas(raw_tx, get_current_eth_addr());
      raw_tx.gas_limit = (*estimated_gas * 125) / 100;
    }

    if (params.contains("maxFeePerGas")) {
      std::string gas_hex = params.at("maxFeePerGas").get<std::string>();
      raw_tx.gas_price = std::stoull(gas_hex, nullptr, 16);
    }

    else if (params.contains("gasPrice")) {
      std::string gas_hex = params.at("gasPrice").get<std::string>();
      raw_tx.gas_price = std::stoull(gas_hex, nullptr, 16);
    } else {

      raw_tx.gas_price =
          static_cast<uint64_t>((gas_manager.get_current_gas() * 115) / 100);
    }

    if (params.contains("chainId")) {
      std::string chain_hex = params.at("chainId").get<std::string>();
      raw_tx.v = std::stoull(chain_hex, nullptr, 16);
    } else {
      raw_tx.v = active_network.chain_id;
    }

    auto res = transaction_manager.send(raw_tx);
    auto [hash, error] = res.get();
    return hash;
  } catch (const std::exception &err) {
    return "";
  }
}

bool BlockchainClient::speed_up_transaction(const bytes_data &private_key) {

  RawTx tx =
      transaction_manager.get_original_tx(tx_status_manager.get_tx_hash());
  tx.gas_price = static_cast<uint64_t>(tx.gas_price * 1.15);
  tx.private_key = private_key;
  tx.v = active_network.chain_id;
  auto rs = transaction_manager.send(tx);
  auto [hash, error] = rs.get();
  if (hash.empty() || error)
    return false;
  tx_status_manager.set_tx_hash(hash);
  return true;
}
bool BlockchainClient::cancel_transaction(const bytes_data &private_key) {
  RawTx tx =
      transaction_manager.get_original_tx(tx_status_manager.get_tx_hash());

  tx.to = tech_utils::from_hex_to_bytes(std::string{get_current_eth_addr()});
  tx.value = Uint256("0", false);
  tx.data = {};
  tx.gas_price = static_cast<uint64_t>(tx.gas_price * 1.15);
  tx.private_key = private_key;
  tx.gas_limit = 21000;
  tx.v = active_network.chain_id;
  auto rs = transaction_manager.send(tx);
  auto [hash, error] = rs.get();
  if (hash.empty() || error)
    return false;
  tx_status_manager.set_tx_hash(hash);
  return true;
}

uint64_t BlockchainClient::get_current_chain_id(void) const {
  return active_network.chain_id;
}

void BlockchainClient::set_get_current_assets_callback(
    std::function<assets_data(uint64_t chain_id)> callback) {
  get_current_assets = callback;
}

Asset BlockchainClient::fetch_new_asset(
    const std::string &contract_addr) const {
  return tokens_metadata_manager
      .fetch_token_metadata_by_contract_addr(contract_addr)
      .value_or(Asset{});
}
