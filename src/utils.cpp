#include "utils.hpp"
#include <stdexcept>
#include <iostream>
#include <numeric>
#include <format>
namespace crypto_utils {

Hashes::Hashes() {
  sha256_ = unique_evp_md(EVP_MD_fetch(nullptr, "SHA256", nullptr));

  if (!sha256_) {
    throw std::runtime_error("sha256 fetching failed");
  }
}

bytes_data Hashes::sha256(const bytes_data &msg) {
bytes_data hash(32);
  unique_evp_md_ctx ctx(EVP_MD_CTX_new());
  if (!ctx) {
    throw std::runtime_error("ctx fetching failed");
  }
  unsigned int len = 0;
  if (!EVP_DigestInit_ex(ctx.get(), sha256_.get(), nullptr)) {
    throw std::runtime_error("digest init failed");
  }

  if (!EVP_DigestUpdate(ctx.get(), msg.data(), msg.size())) {
    throw std::runtime_error("digest update failed");
  }

  if (!EVP_DigestFinal_ex(ctx.get(), hash.data(), &len)) {
    throw std::runtime_error("digest final failed");
  }

  return hash;
}
bytes_data Hashes::PBKDF2_HMAC_SHA512(bytes_data&data,
                                                bytes_data &salt,
                                                int iter) {
  bytes_data out(64);
  int res = PKCS5_PBKDF2_HMAC(reinterpret_cast<char *>(data.data()),
                              static_cast<int>(data.size()),
                              reinterpret_cast<unsigned char *>(salt.data()),
                              static_cast<int>(salt.size()), iter, EVP_sha512(),
                              static_cast<int>(out.size()),
                              reinterpret_cast<unsigned char *>(out.data()));
  if (res == 0) {
    throw std::runtime_error("OpenSSL: PBKDF2_HMAC_SHA512 failed");
  }
  return out;
}

bytes_data getCheckSum(uint8_t byte, int checkSumBits) {
  bytes_data checksum(checkSumBits);

  for (int i = 0; i < checkSumBits; i++) {
    checksum[i] = (byte >> (7 - i)) & 1;
  }

  return checksum;
}

bytes_data HMAC_SHA512(std::string_view key,
                                 const bytes_data &data) {
  bytes_data out(64);
  unsigned int len;
  if (!HMAC(EVP_sha512(), key.data(), static_cast<int>(key.size()), data.data(),
            static_cast<int>(data.size()), out.data(), &len)) {
    throw std::runtime_error("OpenSSL: HMAC_SHA512 failed");
  }

  return out;
}

bytes_data HMAC_SHA512(const bytes_data &key,
                                 const bytes_data &data) {
  bytes_data out(64);
  unsigned int len;
  if (!HMAC(EVP_sha512(), key.data(), static_cast<int>(key.size()), data.data(),
            static_cast<int>(data.size()), out.data(), &len)) {
    throw std::runtime_error("OpenSSL: HMAC_SHA512 failed");
  }

  return out;
}

void split_key(const bytes_data &master_private_key,
               bytes_data &private_key,
               bytes_data &chain_key) {

  if (master_private_key.size() != 64) {
    throw std::invalid_argument("Master key must be exactly 64 bytes");
  }

  private_key.assign(master_private_key.begin(),
                     master_private_key.begin() + 32);
  chain_key.assign(master_private_key.begin() + 32, master_private_key.end());
}

} // namespace crypto_utils

namespace tech_utils {
  void clear_stdin(void) {
    std::cin.clear();

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }

    void print_hex(const bytes_data& data) {
      std::cout << "0x";

      for(const auto & byte : data) {
        std::cout << std::format("{:02x}", byte);
      }
      std::cout << std::endl;
    }

    bytes_data toBits(const std::vector<uint16_t>& data) {
      bytes_data binaryData;
      binaryData.reserve(data.size() * 11);

      for(const auto& byte2 : data) {
        for(int i = 10; i >= 0; i--) {
          binaryData.push_back(byte2 >> i & 1);
        }
      }
      return binaryData;
    }

    bytes_data to_bytes_from_bits(const bytes_data&data) {
      if(data.size() % 8 != 0) {
        return {};
      }

      bytes_data new_data;
      new_data.reserve(data.size() / 8);
      for(size_t i = 0; i < data.size(); i+=8) {
        uint8_t byte = 0;
        for(size_t j = 0; j < 8; j++) {
          byte = (byte << 1) | (data[j + i] & 1);
        }
        new_data.push_back(byte);
      }

      return new_data;
    }
}
