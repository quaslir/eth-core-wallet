#include "drivers/blockchain_client.hpp"
#include "api/http.hpp"
#include "api/json.hpp"
#include "config/configuration.hpp"
#include "core/secure_bytes_data.hpp"
#include "core/uint256.hpp"
#include "utils/tech_utils.hpp"
#include <exception>
#include <string>

double BlockchainClient::get_balance(const std::string &eth_addr) const {

  try {
    GetBalanceMethod alchm("2.0", "eth_getBalance", {eth_addr, "latest"}, 1);

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

std::vector<TransactionRecord>
BlockchainClient::parse_transactions(const json &j, bool incoming) {
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
BlockchainClient::get_transaction_history(const std::string &eth_addr) const {

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

double BlockchainClient::get_eth_price_in_usd(void) {
  try {
    std::string buffer = http::get_request(ETH_USD_URL);
    json j = json::parse(buffer);

    double value = j.value("USD", 0.0);
    return value;

  } catch (const std::exception &err) {
    return 0.0;
  }
}
