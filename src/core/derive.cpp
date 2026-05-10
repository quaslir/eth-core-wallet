#include "core/derive.hpp"
#include "Keccak256.hpp"

#include "core/secure_bytes_data.hpp"
#include "utils/crypto_utils.hpp"
#include "utils/tech_utils.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <ranges>

#include <stdexcept>
#include <string>
extern "C" {
#include <arpa/inet.h>
#include <openssl/bn.h>
}
Key_Derive::Key_Derive() : n(BN_new()) {
  if (!n) {
    throw std::runtime_error("BN_new failed");
  }
  context = secp256k1_context_create(SECP256K1_CONTEXT_SIGN |
                                     SECP256K1_CONTEXT_VERIFY);

  if (!context) {
    throw std::runtime_error("Critical error, memory allocation failed");
  }
  BN_hex2bn(&n,
            "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141");
}

Key_Derive::~Key_Derive() {
  BN_free(n);
  secp256k1_context_destroy(context);
}

bytes_data Key_Derive::derive_public_key(bytes_data &private_key) const {
  secp256k1_pubkey public_key;
  bytes_data compressed_public_key(33);
  size_t len = 33;

  if (!secp256k1_ec_pubkey_create(context, &public_key, private_key.data())) {
    throw std::runtime_error("Invalid private key");
  }

  secp256k1_ec_pubkey_serialize(context, compressed_public_key.data(), &len,
                                &public_key, SECP256K1_EC_COMPRESSED);

  return compressed_public_key;
}

void Key_Derive::derive_child(KEY_PAIR &keys, uint32_t index) {
  bytes_data data;
  data.reserve(37);
  if (index >= 0x80000000) {

    data.push_back(0x00);
    data.insert(data.end(), keys.parent_key.begin(), keys.parent_key.end());
  } else {
    bytes_data pub_key = derive_public_key(keys.parent_key);
    data.insert(data.end(), pub_key.begin(), pub_key.end());
  }
  uint32_t be_index = htonl(index);
  const uint8_t *index_ptr = reinterpret_cast<const uint8_t *>(&be_index);

  data.insert(data.end(), index_ptr, index_ptr + 4);
  bytes_data I = crypto_utils::HMAC_SHA512(keys.chain_key, data);

  keys.parent_key =
      add_mod_n(bytes_data(I.begin(), I.begin() + 32), keys.parent_key);
  keys.chain_key.assign(I.begin() + 32, I.end());
}

bytes_data Key_Derive::add_mod_n(const bytes_data &IL,
                                 const bytes_data &k_parent) const {

  BN_CTX *bn_ctx = BN_CTX_new();

  BIGNUM *bn_IL = BN_bin2bn(IL.data(), static_cast<int>(IL.size()), nullptr);
  BIGNUM *bn_k_parent =
      BN_bin2bn(k_parent.data(), static_cast<int>(k_parent.size()), nullptr);

  BIGNUM *bn_res = BN_new();

  if (!BN_mod_add(bn_res, bn_IL, bn_k_parent, n, bn_ctx)) {
    throw std::runtime_error("mod addition failed");
  }

  bytes_data child_priv(32);

  BN_bn2binpad(bn_res, child_priv.data(), 32);

  BN_free(bn_IL);
  BN_free(bn_k_parent);
  BN_free(bn_res);
  BN_CTX_free(bn_ctx);

  return child_priv;
}

secure_string
Key_Derive::generate_address(const bytes_data &private_key) const {
  secp256k1_pubkey public_key;
  bytes_data uncompressed_public_key(65);
  size_t len = 65;

  if (!secp256k1_ec_pubkey_create(context, &public_key, private_key.data())) {
    throw std::runtime_error("Invalid private key");
  }

  secp256k1_ec_pubkey_serialize(context, uncompressed_public_key.data(), &len,
                                &public_key, SECP256K1_EC_UNCOMPRESSED);

  uint8_t full_hash[32];
  Keccak256::getHash(uncompressed_public_key.data() + 1, len - 1, full_hash);
  bytes_data raw_addr(full_hash + 12, full_hash + 32);
  secure_string eth_address = tech_utils::to_hex(raw_addr);

  std::transform(eth_address.begin(), eth_address.end(), eth_address.begin(),
                 ::tolower);
  return to_checksum_address(eth_address);
}

const std::vector<uint32_t>
Key_Derive::parse_derive_path(const secure_string &path) {
  auto paths = path | std::views::split('/');
  std::vector<uint32_t> derivation_indexes;
  bool start = true;
  for (auto index : paths) {
    std::string index_to_string{index.begin(), index.end()};

    tech_utils::trim(index_to_string);
    if (start && index_to_string == "m") {
      start = false;
      continue;
    }

    bool hardened_derivation = index_to_string.back() == '\'';
    if (hardened_derivation)
      index_to_string.pop_back();
    uint32_t i = tech_utils::parse_uint32(index_to_string);
    if (hardened_derivation)
      i |= 0x80000000;

    derivation_indexes.push_back(i);
  }

  return derivation_indexes;
}

secure_string Key_Derive::to_checksum_address(const secure_string &addr) const {
  secure_string result = "0x";

  uint8_t hash[32];

  Keccak256::getHash(reinterpret_cast<const uint8_t *>(addr.data()),
                     addr.size(), hash);
  for (size_t i = 0; i < addr.size(); i++) {
    char c = addr[i];

    if (c >= '0' && c <= '9') {
      result += c;
      continue;
    }

    uint8_t b = hash[i / 2];

    int nibble = (i % 2 == 0) ? (b >> 4) : (b & 0x0F);

    if (nibble >= 8) {
      result += std::toupper(c);
    }

    else
      result += c;
  }

  return result;
}
