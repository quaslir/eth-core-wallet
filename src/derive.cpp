#include <derive.hpp>
#include <stdexcept>
#include "Keccak256.hpp"
#include "utils.hpp"
extern "C" {
    #include <openssl/bn.h>
    #include <arpa/inet.h>
}
Key_Derive::Key_Derive() : n(BN_new()) 
{
    if(!n) {
        throw std::runtime_error("BN_new failed");
    }
    context = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);

    if(!context) {
        throw std::runtime_error("Critical error, memory allocation failed");
    }
    BN_hex2bn(&n, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141");
}

Key_Derive::~Key_Derive() {
    BN_free(n);
    secp256k1_context_destroy(context);
}

std::vector<uint8_t> Key_Derive::derive_public_key(std::vector<uint8_t>& private_key) {
    secp256k1_pubkey public_key;
    std::vector<uint8_t> compressed_public_key(33);
     size_t len = 33;

    if(!secp256k1_ec_pubkey_create(context, &public_key, private_key.data())) {
        throw std::runtime_error("Invalid private key");
    }

    secp256k1_ec_pubkey_serialize(context, compressed_public_key.data(), &len, &public_key, SECP256K1_EC_COMPRESSED);

    return compressed_public_key;
}

    void Key_Derive::derive_child(std::vector<uint8_t> & parent_key, std::vector<uint8_t>& chain_key, uint32_t index) {
        std::vector<uint8_t> data;
        data.reserve(37);
        if(index >= 0x80000000) {

        data.push_back(0x00);
        data.insert(data.end(), parent_key.begin(), parent_key.end());
        } else {
            std::vector<uint8_t> pub_key = derive_public_key(parent_key);
            data.insert(data.end(), pub_key.begin(), pub_key.end());

        }
        uint32_t be_index = htonl(index);
        const uint8_t * index_ptr = reinterpret_cast<const uint8_t*>(&be_index);

        data.insert(data.end(),index_ptr, index_ptr + 4);
        std::vector<uint8_t> I = crypto_utils::HMAC_SHA512(chain_key, data);

        parent_key = add_mod_n(std::vector<uint8_t>(I.begin(), I.begin() + 32), parent_key);
        chain_key.assign(I.begin() + 32, I.end());
}


std::vector<uint8_t> Key_Derive::add_mod_n(const std::vector<uint8_t>& IL, const std::vector<uint8_t> & k_parent) {

    BN_CTX* bn_ctx = BN_CTX_new();

    BIGNUM* bn_IL =  BN_bin2bn(IL.data(), static_cast<int>(IL.size()), nullptr);
    BIGNUM* bn_k_parent =  BN_bin2bn(k_parent.data(), static_cast<int>(k_parent.size()), nullptr);

    BIGNUM * bn_res = BN_new();

    if(!BN_mod_add(bn_res, bn_IL, bn_k_parent, n, bn_ctx)) {
        throw std::runtime_error("mod addition failed");
    }

    std::vector<uint8_t> child_priv(32);

    BN_bn2binpad(bn_res, child_priv.data(), 32);

    BN_free(bn_IL);
    BN_free(bn_k_parent);
    BN_free(bn_res);
    BN_CTX_free(bn_ctx);

    return child_priv;
}


std::vector<uint8_t> Key_Derive::generate_address(const std::vector<uint8_t> & private_key) {
    secp256k1_pubkey public_key;
    std::vector<uint8_t> uncompressed_public_key(65);
     size_t len = 65;

    if(!secp256k1_ec_pubkey_create(context, &public_key, private_key.data())) {
        throw std::runtime_error("Invalid private key");
    }

    secp256k1_ec_pubkey_serialize(context, uncompressed_public_key.data(), &len, &public_key, SECP256K1_EC_UNCOMPRESSED);

    uint8_t full_hash[32];
    Keccak256::getHash(uncompressed_public_key.data() + 1, len - 1, full_hash);
    std::vector<uint8_t> eth_address(full_hash + 12, full_hash + 32);

    return eth_address;
    
}