#include <secp256k1.h>
#include <vector>
#include <stdint.h>
class Key_Derive {
    private:
    secp256k1_context* context = nullptr;
    BIGNUM *n = nullptr;

    public:
        Key_Derive();
    ~Key_Derive();
    std::vector<uint8_t> derive_public_key(std::vector<uint8_t>& private_key);
    void derive_child(std::vector<uint8_t> & master_key, std::vector<uint8_t>& chain_key, uint32_t index);
    std::vector<uint8_t> add_mod_n(const std::vector<uint8_t>& IL, const std::vector<uint8_t> & k_parent);

};