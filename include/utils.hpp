#pragma once
#include <charconv>
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

struct EVP_CIPHER_CTXDeleter {
  void operator()(EVP_CIPHER_CTX *ptr) const {
    if (ptr) {
      EVP_CIPHER_CTX_free(ptr);
    }
  }
};

using unique_evp_md_ctx = std::unique_ptr<EVP_MD_CTX, EVPMDCTXDeleter>;
using unique_evp_md = std::unique_ptr<EVP_MD, EVPMDDeleter>;
using unique_evp_cipher_ctx =
    std::unique_ptr<EVP_CIPHER_CTX, EVP_CIPHER_CTXDeleter>;

class Hashes {
private:
  unique_evp_md sha256_;

public:
  Hashes();
  bytes_data sha256(const bytes_data &msg);
};
bytes_data PBKDF2_HMAC_SHA512(const bytes_data &data, const bytes_data &salt,
                              int iter);
bytes_data HMAC_SHA512(std::string_view key, const bytes_data &data);
bytes_data HMAC_SHA512(const bytes_data &key, const bytes_data &data);
bytes_data AES_256_CTR(const bytes_data &key, const bytes_data &data,
                       const bytes_data &iv);
bytes_data genNumber(size_t bytes);
bytes_data getCheckSum(uint8_t byte, int checkSumBits);
void split_key(const bytes_data &master_private_key, bytes_data &private_key,
               bytes_data &chain_key); // 64 bytes!

const std::vector<uint32_t> path_deriv = {0x8000002C, 0x8000003C, 0x80000000,
                                          0x00000000, 0x00000000};

bool is_valid_derive_path(const std::string &path);
std::vector<uint32_t> change_derive_path(unsigned int index);
} // namespace crypto_utils

namespace tech_utils {
void clear_stdin(void);
std::string read_stdin(void);
void print_hex(const bytes_data &data);
std::string to_hex(const bytes_data &data);
bytes_data from_hex_to_bytes(const std::string &hex);
bytes_data toBits(const std::vector<uint16_t> &data);
bytes_data to_bytes_from_bits(const bytes_data &data);
uint32_t parse_uint32(const std::string &data);
void trim(std::string &data);
void rm_file(const std::string &filename);
} // namespace tech_utils
