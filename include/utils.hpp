#include <memory>
#include <stdint.h>
#include <vector>
extern "C" {
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
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
};

} // namespace crypto_utils