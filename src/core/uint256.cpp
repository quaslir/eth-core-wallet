#include "core/uint256.hpp"
#include "core/secure_bytes_data.hpp"
#include <cstddef>
#include <openssl/bn.h>

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
      if(BN_is_zero(bn.get())) return {};
      int num_bytes = BN_num_bytes(bn.get());
      bytes_data result(num_bytes);
      BN_bn2bin(bn.get(), result.data());
      return result;
  }
