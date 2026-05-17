#include "drivers/transaction_client.hpp"
#include "api/json.hpp"
#include "api/http.hpp"
#include "core/secure_bytes_data.hpp"
#include "core/uint256.hpp"
#include <cstdint>
#include <exception>
#include <future>
#include <optional>
#include <string>
#include <tuple>
#include "api/rlp.hpp"
#include "Keccak256.hpp"
#include "secp256k1.h"
#include "secp256k1_recovery.h"
#include "utils/tech_utils.hpp"

std::future<std::string> TransactionManager::send(RawTx & tx) {
return std::async(std::launch::async, [this, tx]() -> std::string {
    bytes_data to_sign = rlp::encode_list({
        rlp::encode_uint(tx.nonce),
        rlp::encode_uint(tx.gas_price),
        rlp::encode_uint(tx.gas_limit),
        rlp::encode_bytes(tx.to),
        rlp::encode_bytes(tx.value.to_bytes()),
        rlp::encode_bytes(tx.data),
        rlp::encode_uint(tx.v),
        rlp::encode_uint(0),
        rlp::encode_uint(0)
    });
    bytes_data hash(32);
    Keccak256::getHash(to_sign.data(), to_sign.size(), hash.data());

    auto [r, s, recovery_id] = sign_transaction(hash, tx.private_key);

    uint64_t final_v = tx.v * 2 + 35 + recovery_id;

    bytes_data signed_tx = rlp::encode_list({
        rlp::encode_uint(tx.nonce),
        rlp::encode_uint(tx.gas_price),
        rlp::encode_uint(tx.gas_limit),
        rlp::encode_bytes(tx.to),
        rlp::encode_bytes(tx.value.to_bytes()),
          rlp::encode_bytes(tx.data),
          rlp::encode_uint(final_v),
          rlp::encode_bytes(r),
          rlp::encode_bytes(s)
    });
    return make_request(signed_tx);
});
}


std::optional<uint64_t> TransactionManager::get_nonce(const secure_string& eth_addr, const std::string & url) {
    try {
    AlchemyJSON alchm("2.0", "eth_getTransactionCount", {std::string{eth_addr}, "pending"}, 1);

    std::string buffer = alchm.to_string();

    std::string result = http::post_request(url, buffer);

    json j = json::parse(result);

    std::string value = j.at("result").get<std::string>();

    uint64_t converted = std::stoull(value, nullptr, 16);
    return converted;
    } catch(const std::exception& err) {
        return std::nullopt;
    }
}

std::tuple<bytes_data, bytes_data, int> TransactionManager::sign_transaction(const bytes_data& hash, const bytes_data& key) {
    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    secp256k1_ecdsa_recoverable_signature signature;
    secp256k1_ecdsa_sign_recoverable(ctx, &signature, hash.data(), key.data(), nullptr, nullptr);

    uint8_t output[64];

    int recovery_id;
    secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, output, &recovery_id, &signature);

    secp256k1_context_destroy(ctx);
    return {bytes_data (output, output + 32), bytes_data(output + 32, output + 64), recovery_id};
}

std::string TransactionManager::make_request(const bytes_data& data) const {
    secure_string hex_data = "0x";
    hex_data += tech_utils::to_hex(data);
    json j;
    j["jsonrpc"] = "2.0";
    j["method"] = "eth_sendRawTransaction";
    j["params"] = json::array({hex_data});
    j["id"] = 1;

    std::string result = http::post_request(form_url(), j.dump());
    std::cerr << result;
    json res = json::parse(result);

    return res.at("result").get<std::string>();
}
