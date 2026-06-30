#include "utils/crypto_utils.hpp"
#include "Keccak256.hpp"
#include "core/secure_bytes_data.hpp"
#include "utils/tech_utils.hpp"
#include <cstdint>
#include <regex>
#include <span>
#include <stdexcept>
#include <string>
extern "C" {
#include "secp256k1.h"
#include "secp256k1_recovery.h"
#include <openssl/rand.h>
}
namespace crypto_utils {

Hashes::Hashes() {
  sha256_ = unique_evp_md(EVP_MD_fetch(nullptr, "SHA256", nullptr));

  if (!sha256_) {
    throw std::runtime_error("sha256 fetching failed");
  }
}

bytes_data Hashes::sha256(const bytes_data &msg) const {
  bytes_data hash(32);
  unique_evp_md_ctx ctx(EVP_MD_CTX_new());
  if (!ctx) {
    throw std::runtime_error("ctx fetching failed");
  }
  unsigned int len = 0;
  if (!EVP_DigestInit_ex(ctx.get(), sha256_.get(), nullptr)) {
    throw std::runtime_error("digest init failed");
  }

  if (!EVP_DigestUpdate(ctx.get(), msg.data(), msg.size())) {
    throw std::runtime_error("digest update failed");
  }

  if (!EVP_DigestFinal_ex(ctx.get(), hash.data(), &len)) {
    throw std::runtime_error("digest final failed");
  }

  return hash;
}
bytes_data PBKDF2_HMAC_SHA512(std::span<const char> data,
                              const bytes_data &salt, int iter) {
  bytes_data out(64);
  int res =
      PKCS5_PBKDF2_HMAC(reinterpret_cast<const char *>(data.data()),
                        static_cast<int>(data.size()),
                        reinterpret_cast<const unsigned char *>(salt.data()),
                        static_cast<int>(salt.size()), iter, EVP_sha512(),
                        static_cast<int>(out.size()),
                        reinterpret_cast<unsigned char *>(out.data()));
  if (res == 0) {
    throw std::runtime_error("OpenSSL: PBKDF2_HMAC_SHA512 failed");
  }
  return out;
}

bytes_data getCheckSum(uint8_t byte, int checkSumBits) {
  bytes_data checksum(checkSumBits);

  for (int i = 0; i < checkSumBits; i++) {
    checksum[i] = (byte >> (7 - i)) & 1;
  }

  return checksum;
}

bytes_data HMAC_SHA512(std::string_view key, const bytes_data &data) {
  bytes_data out(64);
  unsigned int len;
  if (!HMAC(EVP_sha512(), key.data(), static_cast<int>(key.size()), data.data(),
            static_cast<int>(data.size()), out.data(), &len)) {
    throw std::runtime_error("OpenSSL: HMAC_SHA512 failed");
  }

  return out;
}

bytes_data HMAC_SHA512(const bytes_data &key, const bytes_data &data) {
  bytes_data out(64);
  unsigned int len;
  if (!HMAC(EVP_sha512(), key.data(), static_cast<int>(key.size()), data.data(),
            static_cast<int>(data.size()), out.data(), &len)) {
    throw std::runtime_error("OpenSSL: HMAC_SHA512 failed");
  }

  return out;
}

bytes_data AES_256_CTR(const bytes_data &key, const bytes_data &data,
                       const bytes_data &iv) {
  unique_evp_cipher_ctx ctx(EVP_CIPHER_CTX_new());

  if (!ctx) {
    throw std::runtime_error("Failed to create EVP context");
  }

  bytes_data ciphertext(data.size());
  int len;

  if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_ctr(), nullptr, key.data(),
                         iv.data()) != 1) {
    throw std::runtime_error("EVP_EncryptInit failed");
  }
  if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len, data.data(),
                        static_cast<int>(data.size())) != 1) {

    throw std::runtime_error("EVP_EncryptUpdate failed");
  }

  int final_len;

  if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len, &final_len) !=
      1) {
    throw std::runtime_error("EVP_EncryptFinal_ex failed");
  }

  return ciphertext;
}

bytes_data gen_number(size_t bytes) {
  bytes_data buf(bytes);

  if (RAND_bytes(buf.data(), static_cast<int>(bytes)) != 1) {
    throw std::runtime_error(
        "OpenSSL: Failed to generate cryptographically strong random bytes.");
  }

  return buf;
}

void split_key_64(const bytes_data &master_private_key, bytes_data &private_key,
                  bytes_data &chain_key) {

  if (master_private_key.size() != 64) {
    throw std::invalid_argument("Master key must be exactly 64 bytes");
  }

  private_key.assign(master_private_key.begin(),
                     master_private_key.begin() + 32);
  chain_key.assign(master_private_key.begin() + 32, master_private_key.end());
}
bool is_valid_derive_path(const std::string &path) {
  static const std::regex bip44_regex(
      R"(^m(\/([0-9]|[1-9][0-9]*)['h]?){1,255}$)");
  return std::regex_match(path, bip44_regex);
}

std::vector<uint32_t> change_derive_path(unsigned int index) {
  if (index >= 0x80000000) {
    throw std::invalid_argument("invalid index");
  }

  std::vector<uint32_t> new_path_deriv(path_deriv.begin(), path_deriv.end());
  new_path_deriv.back() = index;
  return new_path_deriv;
}

std::tuple<bytes_data, bytes_data, int>
sign_transaction(const bytes_data &hash, const bytes_data &key) {
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

secure_string sign_personal_message(const bytes_data &raw_msg_bytes,
                                    const bytes_data &key) {
  std::string prefix = "\x19"
                       "Ethereum Signed Message:\n" +
                       std::to_string(raw_msg_bytes.size());

  bytes_data prefixed_msg;
  prefixed_msg.insert(prefixed_msg.end(), prefix.begin(), prefix.end());
  prefixed_msg.insert(prefixed_msg.end(), raw_msg_bytes.begin(),
                      raw_msg_bytes.end());

  bytes_data hash(32);
  Keccak256::getHash(prefixed_msg.data(), prefixed_msg.size(), hash.data());

  auto [r, s, rec_id] = sign_transaction(hash, key);

  uint8_t v = 27 + rec_id;

  bytes_data full_signature;

  full_signature.insert(full_signature.end(), r.begin(), r.end());
  full_signature.insert(full_signature.end(), s.begin(), s.end());
  full_signature.push_back(v);

  return "0x" + tech_utils::to_hex(full_signature);
}
secure_string sign_typed_data(const eip712::bytes_t& digest, const bytes_data& key) {
    bytes_data hash(digest.begin(), digest.end());
    auto[r, s, recovery_id] = sign_transaction(hash, key);
    uint8_t v= 27 + recovery_id;
    bytes_data signature(r.begin(), r.end());
    signature.insert(signature.end(), s.begin(), s.end());
    signature.push_back(v);

    return "0x" + tech_utils::to_hex(signature);
}
} // namespace crypto_utils
