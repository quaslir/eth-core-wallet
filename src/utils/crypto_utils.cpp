#include "crypto_utils.hpp"
#include <cstdint>
#include <iostream>
#include <regex>
#include <stdexcept>
extern "C" {
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
bytes_data PBKDF2_HMAC_SHA512(const bytes_data &data, const bytes_data &salt,
                              int iter) {
  bytes_data out(64);
  int res =
      PKCS5_PBKDF2_HMAC(reinterpret_cast<const char *>(data.data()),
                        static_cast<int>(data.size()),
                        reinterpret_cast<const unsigned char *>(salt.data()),
                        static_cast<int>(salt.size()), iter, EVP_sha512(),
                        static_cast<int>(out.size()),
                        reinterpret_cast<unsigned char *>(out.data()));
  if (res == 0) {
    OPENSSL_cleanse(out.data(), out.size());
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

} // namespace crypto_utils
