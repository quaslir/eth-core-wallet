#include "core/uint256.hpp"
#include "core/secure_bytes_data.hpp"
#include <cstddef>
#include <cstdint>
#include <openssl/bn.h>
#include <ranges>

Uint256::Uint256() : bn(BN_new()) {
  if (!bn) {
    throw std::runtime_error("BN_failed");
  }
}

Uint256::Uint256(const std::string &str, bool hex) : Uint256() {
  std::string clean;
  if (hex) {
    if (str.substr(0, 2) == "0x")
      clean = str.substr(2);
    else
      clean = str;

    BIGNUM *raw_bn = bn.get();

    if (!BN_hex2bn(&raw_bn, clean.c_str())) {
      throw std::runtime_error("Invalid hex for Uint256");
    }
  } else {
    BIGNUM *raw_bn = bn.get();

    if (!BN_dec2bn(&raw_bn, str.c_str())) {
      throw std::runtime_error("Invalid dec for Uint256");
    }
  }
}

Uint256::Uint256(const Uint256 &num) : Uint256() {
  if (!BN_copy(bn.get(), num.bn.get())) {
    throw std::runtime_error("OpenSSL: BN_copy failed");
  }
}

Uint256::Uint256(Uint256 &&num) noexcept : bn(std::move(num.bn)) {}

Uint256 &Uint256::operator=(const Uint256 &num) {
  if (this != &num) {
    if (!BN_copy(bn.get(), num.bn.get())) {
      throw std::runtime_error("OpenSSL: BN_copy failed");
    }
  }

  return *this;
}

Uint256 &Uint256::operator=(Uint256 &&num) noexcept {
  if (this != &num) {
    bn = std::move(num.bn);
  }

  return *this;
}

bool Uint256::operator==(const Uint256 &num) const {
  return BN_cmp(bn.get(), num.bn.get()) == 0;
}
bool Uint256::operator<(const Uint256 &num) const {
  return BN_cmp(bn.get(), num.bn.get()) < 0;
}
bool Uint256::operator>(const Uint256 &num) const {
  return BN_cmp(bn.get(), num.bn.get()) > 0;
}
bool Uint256::operator<=(const Uint256 &num) const {
  return operator<(num) || operator==(num);
}
bool Uint256::operator>=(const Uint256 &num) const {
  return operator>(num) || operator==(num);
}

Uint256 &Uint256::operator+=(const Uint256 &num) {
  if (!BN_add(bn.get(), bn.get(), num.bn.get())) {
    throw std::runtime_error("OpenSSL: BN_add failed");
  }

  return *this;
}

Uint256 Uint256::operator+(const Uint256 &num) const {
  Uint256 res(*this);
  res += num;
  return res;
}


Uint256 Uint256::operator*(const Uint256& num) const {
    BN_CTX*ctx =  BN_CTX_new();
    Uint256 result;
    BN_mul(result.bn.get(), bn.get(), num.bn.get(), ctx);
    BN_CTX_free(ctx);
    return result;
}

std::string Uint256::from_wei_to_asset(const std::string &dividor) const {
  BN_CTX *ctx = BN_CTX_new();
  if (!ctx) {
    throw std::runtime_error("OpenSSL: BN_CTX_new failed");
  }
  BIGNUM *raw_divisor = nullptr;

  if (!BN_dec2bn(&raw_divisor, dividor.c_str())) {
    BN_CTX_free(ctx);
    throw std::runtime_error("OpenSSL: BN_dec2bn failed for divisor");
  }
  unique_bignum divisor(raw_divisor);
  unique_bignum result(BN_new());
  unique_bignum remainder(BN_new());

  if (!BN_div(result.get(), remainder.get(), bn.get(), divisor.get(), ctx)) {
    BN_CTX_free(ctx);
    throw std::runtime_error("OpenSSL: BN_div failed");
  }

  char *whole_ptr = BN_bn2dec(result.get());
  char *frac_ptr = BN_bn2dec(remainder.get());
  if (!whole_ptr || !frac_ptr) {
    if (whole_ptr)
      OPENSSL_free(whole_ptr);
    if (frac_ptr)
      OPENSSL_free(frac_ptr);
    BN_CTX_free(ctx);
    throw std::runtime_error("OpenSSL: BN_bn2dec failed");
  }

  std::string whole_ptr_str{whole_ptr};
  std::string frac_ptr_str{frac_ptr};

  size_t precision = dividor.length() - 1;

  if (frac_ptr_str.length() < precision) {
    frac_ptr_str.insert(0, precision - frac_ptr_str.length(), '0');
  }

  OPENSSL_free(whole_ptr);
  OPENSSL_free(frac_ptr);

  BN_CTX_free(ctx);

  return whole_ptr_str + "." + frac_ptr_str.substr(0, 4);
}

bytes_data Uint256::to_bytes(void) const {
  if (BN_is_zero(bn.get()))
    return {};
  int num_bytes = BN_num_bytes(bn.get());
  bytes_data result(num_bytes);
  BN_bn2bin(bn.get(), result.data());
  return result;
}

bytes_data Uint256::to_bytes32(void) const {
    bytes_data result(32, 0x00);
    int num_bytes = BN_num_bytes(bn.get());

    BN_bn2bin(bn.get(), result.data() + (32 - num_bytes));
    return result;
}
Uint256 Uint256::from_decimal_string(const std::string& str, uint8_t decimals) {
    std::string int_part, frac_part;
    size_t dot = str.find(".");
    if(dot == std::string::npos) {
        int_part = str;
        frac_part = "";
    }

    else {
        int_part = str.substr(0, dot);
        frac_part = str.substr(dot + 1);
    }

    if(frac_part.size() > decimals) {
        frac_part = frac_part.substr(0, decimals);
    }
    while(frac_part.size() < decimals) {
        frac_part += "0";
    }

    std::string combined = int_part + frac_part;
    return Uint256(combined, false);
}
