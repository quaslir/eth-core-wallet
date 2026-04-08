#include "uint256.hpp"

Uint256::Uint256() : bn(BN_new()) {
    if(!bn) {
        throw std::runtime_error("BN_failed");
    }
}

Uint256::Uint256(const std::string& str, bool hex) : Uint256() {
std::string clean;
if(hex) {
if(str.substr(0 ,2) == "0x") clean = str.substr(2);
else clean = str;

BIGNUM *raw_bn = bn.get();

if(!BN_hex2bn(&raw_bn, clean.c_str())) {
    throw std::runtime_error("Invalid hex for Uint256");
}
}
else {
    BIGNUM *raw_bn = bn.get();

if(!BN_dec2bn(&raw_bn, str.c_str())) {
    throw std::runtime_error("Invalid dec for Uint256");
}
}
}

Uint256::Uint256(const Uint256& num) : Uint256() {
if(!BN_copy(bn.get(), num.bn.get())) {
    throw std::runtime_error("OpenSSL: BN_copy failed");
}
}

Uint256::Uint256(Uint256&& num) noexcept : bn(std::move(num.bn)) {}

Uint256& Uint256::operator=(const Uint256& num) {
    if(this != &num) {
        if(!BN_copy(bn.get(), num.bn.get())) {
             throw std::runtime_error("OpenSSL: BN_copy failed");
        }
    }

    return *this;
}

Uint256& Uint256::operator=(Uint256&& num) noexcept {
if(this != &num) {
    bn = std::move(num.bn);
}

return *this;
}

bool Uint256::operator==(const Uint256& num) const {
    return BN_cmp(bn.get(), num.bn.get()) == 0;
}
    bool Uint256::operator<(const Uint256& num) const {
        return BN_cmp(bn.get(), num.bn.get()) < 0;
    }
    bool Uint256::operator>(const Uint256& num) const {
        return BN_cmp(bn.get(), num.bn.get()) > 0;
    }
    bool Uint256::operator<=(const Uint256& num) const {
        return operator<(num) || operator==(num);
    }
    bool Uint256::operator>=(const Uint256& num) const {
        return operator>(num) || operator==(num);
    }

    Uint256& Uint256::operator+=(const Uint256& num)  {
        if(!BN_add(bn.get(), bn.get(), num.bn.get())) {
            throw std::runtime_error("OpenSSL: BN_add failed");
        }

        return *this;
    }

    Uint256 Uint256::operator+(const Uint256& num) const {
        Uint256 res(*this);
        res += num;
        return res;
    }

std::string Uint256::from_wei_to_eth(void) const {
BN_CTX * ctx = BN_CTX_new();
BIGNUM * raw_divisor = nullptr;


if(!BN_dec2bn(&raw_divisor, "1000000000000000000")) {
    BN_CTX_free(ctx);
    BN_free(raw_divisor);
}
unique_bignum divisor(raw_divisor);
unique_bignum result(BN_new());
unique_bignum remainder(BN_new());

BN_div(result.get(), remainder.get(), bn.get(), divisor.get(), ctx);

char *whole_ptr = BN_bn2dec(result.get());
char * frac_ptr = BN_bn2dec(remainder.get());

std::string whole_ptr_str{whole_ptr};
std::string frac_ptr_str{frac_ptr};

if(frac_ptr_str.length() < 18) {
    frac_ptr_str.insert(0, 18 - frac_ptr_str.length(), '0');
}

OPENSSL_free(whole_ptr);
OPENSSL_free(frac_ptr);

BN_CTX_free(ctx);

return whole_ptr_str + "." + frac_ptr_str.substr(0, 4);

}