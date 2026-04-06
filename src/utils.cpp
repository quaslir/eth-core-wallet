#include "utils.hpp"
#include <format>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <regex>

extern "C" {
  #include <openssl/rand.h>
}
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
bytes_data PBKDF2_HMAC_SHA512(const bytes_data &data, const bytes_data &salt,
                                      int iter) {
  bytes_data out(64);
  int res = PKCS5_PBKDF2_HMAC(reinterpret_cast<const char *>(data.data()),
                              static_cast<int>(data.size()),
                              reinterpret_cast<const unsigned char *>(salt.data()),
                              static_cast<int>(salt.size()), iter, EVP_sha512(),
                              static_cast<int>(out.size()),
                              reinterpret_cast<unsigned char *>(out.data()));
  if (res == 0) {
    OPENSSL_cleanse(out.data(), out.size());
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

bytes_data HMAC_SHA512(std::string_view key, const bytes_data &data) {
  bytes_data out(64);
  unsigned int len;
  if (!HMAC(EVP_sha512(), key.data(), static_cast<int>(key.size()), data.data(),
            static_cast<int>(data.size()), out.data(), &len)) {
    throw std::runtime_error("OpenSSL: HMAC_SHA512 failed");
  }

  return out;
}

bytes_data HMAC_SHA512(const bytes_data &key, const bytes_data &data) {
  bytes_data out(64);
  unsigned int len;
  if (!HMAC(EVP_sha512(), key.data(), static_cast<int>(key.size()), data.data(),
            static_cast<int>(data.size()), out.data(), &len)) {
    throw std::runtime_error("OpenSSL: HMAC_SHA512 failed");
  }

  return out;
}

bytes_data AES_256_CTR(const bytes_data& key, const bytes_data& data, const bytes_data& iv) {
 unique_evp_cipher_ctx ctx(EVP_CIPHER_CTX_new());

 if(!ctx) {
  throw std::runtime_error("Failed to create EVP context");
 }

 bytes_data ciphertext(data.size());
 int len;

 if(EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_ctr(), nullptr, key.data(), iv.data()) != 1) {
  throw std::runtime_error("EVP_EncryptInit failed");
 }
 if(EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len, data.data(), static_cast<int>(data.size())) != 1) {
 
  throw std::runtime_error("EVP_EncryptUpdate failed");
 }

 int final_len;

 if(EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len, &final_len) != 1) {
  throw std::runtime_error("EVP_EncryptFinal_ex failed");
 }

   return ciphertext;
}

bytes_data genNumber(size_t bytes) {
  bytes_data buf(bytes);

  if (RAND_bytes(buf.data(), static_cast<int>(bytes)) != 1) {
    throw std::runtime_error(
        "OpenSSL: Failed to generate cryptographically strong random bytes.");
  }

  return buf;
}


void split_key(const bytes_data &master_private_key, bytes_data &private_key,
               bytes_data &chain_key) {

  if (master_private_key.size() != 64) {
    throw std::invalid_argument("Master key must be exactly 64 bytes");
  }

  private_key.assign(master_private_key.begin(),
                     master_private_key.begin() + 32);
  chain_key.assign(master_private_key.begin() + 32, master_private_key.end());
}
bool is_valid_derive_path(const std::string& path) {
static const std::regex bip44_regex(R"(^m(\/([0-9]|[1-9][0-9]*)['h]?){1,255}$)");
return std::regex_match(path, bip44_regex);
}
} // namespace crypto_utils

namespace tech_utils {
void clear_stdin(void) {
  std::cin.clear();

  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string read_stdin(void) {
  std::string input;
  if(!std::getline(std::cin, input)) {
    exit(0);
  }
  return input;
}


void print_hex(const bytes_data &data) {
  std::cout << "0x";

  for (const auto &byte : data) {
    std::cout << std::format("{:02x}", byte);
  }
  std::cout << std::endl;
}

std::string to_hex(const bytes_data& data) {
  std::string hex_format;
  hex_format.reserve(data.size());

  for(const auto &byte : data) {
    hex_format.append(std::format("{:02x}", byte));
  }

  return hex_format;
}

bytes_data from_hex_to_bytes(const std::string& hex) {
  if(hex.length() % 2 != 0) {
    throw std::runtime_error("Invalid hex string");
  }

  bytes_data bytes;
  bytes.reserve(hex.length() / 2);

  for(size_t i = 0; i < hex.length(); i+=2) {
    std::string bytesStr = hex.substr(i, 2);

    uint8_t byte = static_cast<uint8_t>(std::stoul(bytesStr, nullptr, 16));

    bytes.push_back(byte);
  }

  return bytes;
}

bytes_data toBits(const std::vector<uint16_t> &data) {
  bytes_data binaryData;
  binaryData.reserve(data.size() * 11);

  for (const auto &byte2 : data) {
    for (int i = 10; i >= 0; i--) {
      binaryData.push_back(byte2 >> i & 1);
    }
  }
  return binaryData;
}

bytes_data to_bytes_from_bits(const bytes_data &data) {
  if (data.size() % 8 != 0) {
    return {};
  }

  bytes_data new_data;
  new_data.reserve(data.size() / 8);
  for (size_t i = 0; i < data.size(); i += 8) {
    uint8_t byte = 0;
    for (size_t j = 0; j < 8; j++) {
      byte = (byte << 1) | (data[j + i] & 1);
    }
    new_data.push_back(byte);
  }

  return new_data;
}


uint32_t parse_uint32(const std::string&data)  {
uint32_t result = 0;

std::from_chars(data.data(), data.data() + data.size(), result);
return result;
}

void trim(std::string& data) {
  size_t end = data.find_last_not_of(" \t\r\n");

  if(end == std::string::npos) {
    data.clear();
    return;
  }
    data.erase(end + 1);
  size_t start = data.find_first_not_of(" \t\r\n");
 
  data.erase(0, start);
}
} // namespace tech_utils
