#pragma once
#include <memory>
#include <stdint.h>
#include <vector>
extern "C" {
#include <openssl/evp.h>
#include <openssl/hmac.h>
}
using bytes_data = std::vector<uint8_t>;
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
  bytes_data sha256(const bytes_data &msg);
  static bytes_data PBKDF2_HMAC_SHA512(bytes_data &data,
                                                 bytes_data &salt,
                                                 int iter);
};
bytes_data HMAC_SHA512(std::string_view key,
                                 const bytes_data &data);
bytes_data HMAC_SHA512(const std::vector<uint8_t> &key,
                                 const bytes_data &data);
std::vector<bool> getCheckSum(uint8_t byte, int checkSumBits);
void split_key(const bytes_data &master_private_key,
               bytes_data &private_key,
               bytes_data &chain_key);

const std::vector<uint32_t> path_deriv = {0x8000002C, 0x8000003C, 0x80000000,
                                          0x00000000, 0x00000000};
} // namespace crypto_utils

namespace tech_utils {
  void clear_stdin(void);
  void print_hex(const std::vector<uint8_t>& data);
}
