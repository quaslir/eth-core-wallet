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
  std::vector<uint8_t> PBKDF2_HMAC_SHA512(std::vector<uint8_t>  &data, std::vector<uint8_t>  & salt, int iter) const; 
};

std::vector<bool> getCheckSum(uint8_t byte, int checkSumBits);

} // namespace crypto_utils
