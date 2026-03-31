#include "mnemonic.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include "bip39.hpp"
MnemonicGenerator::MnemonicGenerator() {}

std::vector<uint8_t> MnemonicGenerator::createMnemonic(std::vector<uint8_t> &&randNumber,
                                  std::vector<bool>&&checkSum){
std::vector<bool> seed;
seed.reserve(randNumber.size() * 8 + checkSum.size());

for(const auto & byte : randNumber) {
  for(int i = 7; i >= 0;i--) {
    seed.push_back((byte >> i) & 1);
  }
}
seed.insert(seed.end(), checkSum.begin(), checkSum.end());
std::vector<uint16_t> indexes;

for(size_t i = 0; i < seed.size(); i+=11) {
  uint16_t index = 0;

  for(int j = 0; j < 11; j++) {
    index = (index << 1) | seed[i + j];
  }

  indexes.push_back(index);
}

OPENSSL_cleanse(randNumber.data(), randNumber.size());
std::vector<std::uint8_t> mnemonic;

for(size_t i = 0; i < indexes.size(); i++) {
  std::string_view word = bip_39::bip_39_words[indexes[i]];
  mnemonic.insert(mnemonic.end(), word.begin(), word.end());
  if(i + 1 < indexes.size()) {
    mnemonic.push_back(static_cast<uint8_t>(' '));
  }
}

OPENSSL_cleanse(indexes.data(), indexes.size() * sizeof(uint16_t));
return mnemonic;
}

std::vector<uint8_t> MnemonicGenerator::generateSeed(uint16_t bits) {
  size_t bytes = bits / 8;
  int checkSumBits = bits / 32;
  std::vector<uint8_t> randNumber = genNumber(bytes);
  std::vector<uint8_t> hash = hashes.sha256(randNumber);

  std::vector<bool> checksum = crypto_utils::getCheckSum(hash[0], checkSumBits);
  OPENSSL_cleanse(hash.data(), hash.size());
  std::vector<uint8_t> mnemonic = createMnemonic(std::move(randNumber),std::move(checksum));

  std::vector<uint8_t> salt = createSalt();

  std::vector<uint8_t> masterseed = hashes.PBKDF2_HMAC_SHA512(mnemonic, salt, 2048);
  OPENSSL_cleanse(mnemonic.data(), mnemonic.size());
  return masterseed;
}

std::vector<uint8_t> MnemonicGenerator::genNumber(size_t bytes) const {
  std::vector<uint8_t> buf(bytes);

  if (RAND_bytes(buf.data(), static_cast<int>(bytes)) != 1) {
    throw std::runtime_error(
        "OpenSSL: Failed to generate cryptographically strong random bytes.");
  }

  return buf;
}

std::vector<uint8_t> MnemonicGenerator::createSalt(void) {
std::string_view view = "mnemonic";
std::vector<uint8_t> salt;

salt.insert(salt.end(), view.begin(), view.end());

return salt;
}