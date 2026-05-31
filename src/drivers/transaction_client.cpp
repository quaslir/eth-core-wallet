#include "drivers/transaction_client.hpp"
#include "Keccak256.hpp"
#include "api/http.hpp"
#include "api/json.hpp"
#include "api/rlp.hpp"
#include "core/secure_bytes_data.hpp"
#include "core/uint256.hpp"
#include "secp256k1.h"
#include "secp256k1_recovery.h"
#include "utils/tech_utils.hpp"
#include <cstdint>
#include <exception>
#include <future>
#include <iostream>
#include <optional>
#include <string>
#include <tuple>
std::future<std::string> TransactionManager::send(RawTx &tx) {
  return std::async(std::launch::async, [this, tx]() -> std::string {
    bytes_data to_sign = rlp::encode_list(
        {rlp::encode_uint(tx.nonce), rlp::encode_uint(tx.gas_price),
         rlp::encode_uint(tx.gas_limit), rlp::encode_bytes(tx.to),
         rlp::encode_bytes(tx.value.to_bytes()), rlp::encode_bytes(tx.data),
         rlp::encode_uint(tx.v), rlp::encode_uint(0), rlp::encode_uint(0)});
    bytes_data hash(32);
    Keccak256::getHash(to_sign.data(), to_sign.size(), hash.data());

    auto [r, s, recovery_id] = sign_transaction(hash, tx.private_key);

    uint64_t final_v = tx.v * 2 + 35 + recovery_id;

    bytes_data signed_tx = rlp::encode_list(
        {rlp::encode_uint(tx.nonce), rlp::encode_uint(tx.gas_price),
         rlp::encode_uint(tx.gas_limit), rlp::encode_bytes(tx.to),
         rlp::encode_bytes(tx.value.to_bytes()), rlp::encode_bytes(tx.data),
         rlp::encode_uint(final_v), rlp::encode_bytes(r),
         rlp::encode_bytes(s)});
    return make_request(signed_tx);
  });
}

std::optional<uint64_t>
TransactionManager::get_nonce(const secure_string &eth_addr,
                              const std::string &url) {
  try {
    AlchemyJSON alchm("2.0", "eth_getTransactionCount",
                      {std::string{eth_addr}, "pending"}, 1);

    std::string buffer = alchm.to_string();

    std::string result = http::post_request(url, buffer);

    json j = json::parse(result);

    std::string hex = j.at("result").get<std::string>();

    return tech_utils::parse_hex(hex);
  } catch (const std::exception &err) {
    return std::nullopt;
  }
}

std::tuple<bytes_data, bytes_data, int>
TransactionManager::sign_transaction(const bytes_data &hash,
                                     const bytes_data &key) {
  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
  secp256k1_ecdsa_recoverable_signature signature;
  secp256k1_ecdsa_sign_recoverable(ctx, &signature, hash.data(), key.data(),
                                   nullptr, nullptr);

  uint8_t output[64];

  int recovery_id;
  secp256k1_ecdsa_recoverable_signature_serialize_compact(
      ctx, output, &recovery_id, &signature);

  secp256k1_context_destroy(ctx);
  return {bytes_data(output, output + 32), bytes_data(output + 32, output + 64),
          recovery_id};
}

std::string TransactionManager::make_request(const bytes_data &data) const {
  try {
    secure_string hex_data = "0x";
    hex_data += tech_utils::to_hex(data);
    json j;
    j["jsonrpc"] = "2.0";
    j["method"] = "eth_sendRawTransaction";
    j["params"] = json::array({hex_data});
    j["id"] = 1;

    std::string result = http::post_request(form_url(), j.dump());
    json res = json::parse(result);

    return res.at("result").get<std::string>();
  } catch (const std::exception &err) {
    return "";
  }
}

bytes_data
TransactionManager::make_transfer_token_data(const bytes_data &to,
                                             const Uint256 &amount) const {
  bytes_data data;
  bytes_data selector = {0xa9, 0x05, 0x9c, 0xbb};
  data.insert(data.end(), selector.begin(), selector.end());

  bytes_data addr_padded(12, 0x00);
  addr_padded.insert(addr_padded.end(), to.begin(), to.end());
  data.insert(data.end(), addr_padded.begin(), addr_padded.end());

  bytes_data amount_padded = amount.to_bytes32();

  data.insert(data.end(), amount_padded.begin(), amount_padded.end());

  return data;
}

std::optional<uint64_t>
TransactionManager::estimate_gas(const RawTx &raw_tx,
                                 const secure_string &from) const {
  try {
    json params;
    params["to"] = "0x" + tech_utils::to_hex(raw_tx.to);
    params["data"] = "0x" + tech_utils::to_hex(raw_tx.data);
    params["from"] = from;

    bytes_data val_bytes = raw_tx.value.to_bytes();

    if (!val_bytes.empty()) {
      params["value"] = "0x" + tech_utils::to_hex(val_bytes);
    }

    json j;
    j["jsonrpc"] = "2.0";
    j["method"] = "eth_estimateGas";
    j["params"] = json::array({params});
    j["id"] = 1;
    std::string data = j.dump();
    std::string result = http::post_request(form_url(), data);
    json res = json::parse(result);
    if (res.contains("error"))
      return std::nullopt;

    std::string hex = res.at("result").get<std::string>();
    return std::stoull(hex, nullptr, 16);

  } catch (const std::exception &err) {
    return std::nullopt;
  }
}

RawTx TransactionManager::get_original_tx(const std::string &tx_hash) const {
  try {
    json j;
    j["jsonrpc"] = "2.0";
    j["method"] = "eth_getTransactionByHash";
    j["params"] = json::array({tx_hash});
    j["id"] = 1;

    std::string buffer = j.dump();
    std::string result = http::post_request(form_url(), buffer);
    json res = json::parse(result);
    json tx = res.at("result").get<json>();

    RawTx raw_tx;
    raw_tx.nonce = std::stoull(tx.at("nonce").get<std::string>(), nullptr, 16);
    raw_tx.gas_price =
        std::stoull(tx.at("gasPrice").get<std::string>(), nullptr, 16);
    raw_tx.gas_limit =
        std::stoull(tx.at("gas").get<std::string>(), nullptr, 16);
    raw_tx.to = tech_utils::from_hex_to_bytes(tx.at("to").get<std::string>());
    raw_tx.value = Uint256(tx.at("value").get<std::string>(), true);

    std::string input = tx.at("input").get<std::string>();

    if (input != "0x") {
      raw_tx.data = tech_utils::from_hex_to_bytes(input);
    }

    return raw_tx;
  } catch (const std::exception &err) {
    return RawTx();
  }
}
