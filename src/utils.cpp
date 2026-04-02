#include "utils.hpp"
#include <stdexcept>
namespace crypto_utils {

Hashes::Hashes() {
  sha256_ = unique_evp_md(EVP_MD_fetch(nullptr, "SHA256", nullptr));

  if (!sha256_) {
    throw std::runtime_error("sha256 fetching failed");
  }
}

std::vector<uint8_t> Hashes::sha256(const std::vector<uint8_t> &msg) {
  std::vector<uint8_t> hash(32);
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
std::vector<uint8_t> Hashes::PBKDF2_HMAC_SHA512(std::vector<uint8_t> &data,
                                                std::vector<uint8_t> &salt,
                                                int iter) {
  std::vector<uint8_t> out(64);
  int res = PKCS5_PBKDF2_HMAC(reinterpret_cast<char *>(data.data()),
                              static_cast<int>(data.size()),
                              reinterpret_cast<unsigned char *>(salt.data()),
                              static_cast<int>(salt.size()), iter, EVP_sha512(),
                              static_cast<int>(out.size()),
                              reinterpret_cast<unsigned char *>(out.data()));
  if (res == 0) {
    throw std::runtime_error("OpenSSL: PBKDF2_HMAC_SHA512 failed");
  }
  return out;
}

std::vector<bool> getCheckSum(uint8_t byte, int checkSumBits) {
  std::vector<bool> checksum(checkSumBits);

  for (int i = 0; i < checkSumBits; i++) {
    checksum[i] = (byte >> (7 - i)) & 1;
  }

  return checksum;
}

std::vector<uint8_t> HMAC_SHA512(std::string_view key,
                                 const std::vector<uint8_t> &data) {
  std::vector<uint8_t> out(64);
  unsigned int len;
  if (!HMAC(EVP_sha512(), key.data(), static_cast<int>(key.size()), data.data(),
            static_cast<int>(data.size()), out.data(), &len)) {
    throw std::runtime_error("OpenSSL: HMAC_SHA512 failed");
  }

  return out;
}

std::vector<uint8_t> HMAC_SHA512(const std::vector<uint8_t> &key,
                                 const std::vector<uint8_t> &data) {
  std::vector<uint8_t> out(64);
  unsigned int len;
  if (!HMAC(EVP_sha512(), key.data(), static_cast<int>(key.size()), data.data(),
            static_cast<int>(data.size()), out.data(), &len)) {
    throw std::runtime_error("OpenSSL: HMAC_SHA512 failed");
  }

  return out;
}

void split_key(const std::vector<uint8_t> &master_private_key,
               std::vector<uint8_t> &private_key,
               std::vector<uint8_t> &chain_key) {

  if (master_private_key.size() != 64) {
    throw std::invalid_argument("Master key must be exactly 64 bytes");
  }

  private_key.assign(master_private_key.begin(),
                     master_private_key.begin() + 32);
  chain_key.assign(master_private_key.begin() + 32, master_private_key.end());
}
} // namespace crypto_utils