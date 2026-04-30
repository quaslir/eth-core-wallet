#include <cstdint>
#include "core/secure_bytes_data.hpp"
#include <string>
#include <vector>
extern "C" {
#include <openssl/bn.h>
#include <secp256k1.h>
}


struct KEY_PAIR {
  bytes_data parent_key, chain_key;

  KEY_PAIR() {}
};

class Key_Derive {
private:
  secp256k1_context *context = nullptr;
  BIGNUM *n = nullptr;

public:
  Key_Derive();
  ~Key_Derive();

  void derive_child(KEY_PAIR &keys, uint32_t index);
  std::string generate_address(const bytes_data &private_key) const;

  static const std::vector<uint32_t> parse_derive_path(const std::string &path);

private:
  bytes_data add_mod_n(const bytes_data &IL, const bytes_data &k_parent) const;
  bytes_data derive_public_key(bytes_data &private_key) const;
  std::string to_checksum_address(const std::string &addr) const;
};
