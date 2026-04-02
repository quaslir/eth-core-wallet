#pragma once
#include <memory>
#include <stdint.h>
#include <vector>
extern "C" {
#include <openssl/evp.h>
#include <openssl/hmac.h>
}

namespace crypto_utils {

struct EVPMDCTXDeleter {
  void operator()(EVP_MD_CTX *ptr) const {
    if (ptr)
      EVP_MD_CTX_free(ptr);
  }
};

struct EVPMDDeleter {
  void operator()(EVP_MD *ptr) const {
    if (ptr)
      EVP_MD_free(ptr);
  }
};

using unique_evp_md_ctx = std::unique_ptr<EVP_MD_CTX, EVPMDCTXDeleter>;
using unique_evp_md = std::unique_ptr<EVP_MD, EVPMDDeleter>;

class Hashes {
private:
  unique_evp_md sha256_;

public:
  Hashes();
  std::vector<uint8_t> sha256(const std::vector<uint8_t> &msg);
  static std::vector<uint8_t> PBKDF2_HMAC_SHA512(std::vector<uint8_t> &data,
                                                 std::vector<uint8_t> &salt,
                                                 int iter);
};
std::vector<uint8_t> HMAC_SHA512(std::string_view key,
                                 const std::vector<uint8_t> &data);
std::vector<uint8_t> HMAC_SHA512(const std::vector<uint8_t> &key,
                                 const std::vector<uint8_t> &data);
std::vector<bool> getCheckSum(uint8_t byte, int checkSumBits);
void split_key(const std::vector<uint8_t> &master_private_key,
               std::vector<uint8_t> &private_key,
               std::vector<uint8_t> &chain_key);

const std::vector<uint32_t> path_deriv = {0x8000002C, 0x8000003C, 0x80000000,
                                          0x00000000, 0x00000000};
} // namespace crypto_utils
