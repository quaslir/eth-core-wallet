#include "blockchain_client.hpp"
#include "http.hpp"
#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include "json.hpp"
std::string BlockchainClient::get_balance(const std::string &eth_addr) const {

  try {
    GetBalanceMethod alchm("2.0", "eth_getBalance", {eth_addr, "latest"}, 1);

    std::string data = alchm.to_string();

    std::string buffer = http::post_request(
        "https://eth-sepolia.g.alchemy.com/v2/MkveNSvN4rHOvLoZK8dE3", data);

    if (buffer.empty())
      return "";

    alchm.parse(buffer);

    Uint256 uint256_t(alchm.get_result(), 1);

    return uint256_t.from_wei_to_eth();

  } catch (const std::exception &err) { // handle in the future
   //handle
    return "";
  }
}

std::vector<TransactionRecord>BlockchainClient:: parse_transactions(const json& j) const {
    try {
std::vector<TransactionRecord> history;

for(const auto& item: j["result"]["transfers"]) {
    TransactionRecord tx;
    tx.hash = item["hash"].get<std::string>();
    tx.value = item.is_null() ? 0.0 : item["value"].get<double>();
    tx.asset = item["asset"].get<std::string>();

    if(item.contains("from")) {
        tx.from_to = item["from"].get<std::string>();
    }

    if(item.contains("metadata") && !item["metadata"]["blockTimestamp"].is_null()) {
        tx.timestamp = item["metadata"]["blockTimestamp"].get<std::string>();
    }

    return history;
}
    } catch(const std::exception& err) {
        return {};
    }

    return {};
}


void BlockchainClient::get_transaction_history(const std::string& eth_addr) const {
json request_body_1 = {
    {"id", 1},
    {"jsonrpc", "2.0"},
    {"method", "alchemy_getAssetTransfers"},
    {"params", {
        {
            {"fromBlock", "0x0"},
            {"toBlock", "latest"},
            {"toAddress", eth_addr},
            {"category", {"external", "erc20"}},
            {"withMetadata", true},
            {"excludeZeroValue", true}
        }
    }}
};

try {
    std::string data = request_body_1.dump();
    std::string buffer = http::post_request("https://eth-sepolia.g.alchemy.com/v2/MkveNSvN4rHOvLoZK8dE3", data);
    json res = json::parse(buffer);
    auto parsed = parse_transactions(res);
    if(parsed.empty()) {
        // handle
    }


} catch(const std::exception& err) {
    // handle
}
}
