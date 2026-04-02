#include <stdint.h>
#include <vector>
extern "C" {
#include <openssl/bn.h>
#include <secp256k1.h>
}

using bytes_data = std::vector<uint8_t>;

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
  bytes_data generate_address(const bytes_data &private_key);

private:
  bytes_data add_mod_n(const bytes_data &IL, const bytes_data &k_parent);
  bytes_data derive_public_key(bytes_data &private_key);
};