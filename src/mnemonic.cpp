#include "mnemonic.hpp"
#include "bip39.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <ranges>
MnemonicGenerator::MnemonicGenerator() {}

bytes_data
MnemonicGenerator::createMnemonic(bytes_data &randNumber,
                                  bytes_data& checkSum) {
  bytes_data seed;
  seed.reserve(randNumber.size() * 8 + checkSum.size());

  for (const auto &byte : randNumber) {
    for (int i = 7; i >= 0; i--) {
      seed.push_back((byte >> i) & 1);
    }
  }
  seed.insert(seed.end(), checkSum.begin(), checkSum.end());
  OPENSSL_cleanse(randNumber.data(), randNumber.size());
  OPENSSL_cleanse(checkSum.data(), checkSum.size());
  std::vector<uint16_t> indexes;

  for (size_t i = 0; i < seed.size(); i += 11) {
    uint16_t index = 0;

    for (int j = 0; j < 11; j++) {
      index = (index << 1) | seed[i + j];
    }

    indexes.push_back(index);
  }
OPENSSL_cleanse(seed.data(), seed.size());
  bytes_data mnemonic;

  for (size_t i = 0; i < indexes.size(); i++) {
    std::string_view word = bip_39::bip_39_words[indexes[i]];
    mnemonic.insert(mnemonic.end(), word.begin(), word.end());
    if (i + 1 < indexes.size()) {
      mnemonic.push_back(static_cast<uint8_t>(' '));
    }
  }

  OPENSSL_cleanse(indexes.data(), indexes.size() * sizeof(uint16_t));
  return mnemonic;
}

bytes_data MnemonicGenerator::generateMnemonic(uint16_t bits) {
  size_t bytes = bits / 8;
  int checkSumBits = bits / 32;
bytes_data randNumber = genNumber(bytes);
bytes_data hash = hashes.sha256(randNumber);

  bytes_data checksum = crypto_utils::getCheckSum(hash[0], checkSumBits);
  OPENSSL_cleanse(hash.data(), hash.size());
  bytes_data mnemonic = createMnemonic(randNumber, checksum);
  return mnemonic;
}

bytes_data MnemonicGenerator::generateSeed(bytes_data & mnemonic,  bytes_data& passphrase) {
bytes_data salt = createSalt(passphrase);

 bytes_data masterseed =
      hashes.PBKDF2_HMAC_SHA512(mnemonic, salt, 2048);
  OPENSSL_cleanse(mnemonic.data(), mnemonic.size());
  OPENSSL_cleanse(passphrase.data(), passphrase.size());
  return masterseed;
}

bytes_data MnemonicGenerator::genNumber(size_t bytes) {
bytes_data buf(bytes);

  if (RAND_bytes(buf.data(), static_cast<int>(bytes)) != 1) {
    throw std::runtime_error(
        "OpenSSL: Failed to generate cryptographically strong random bytes.");
  }

  return buf;
}

bytes_data
MnemonicGenerator::createSalt(bytes_data& passphrase) {
  std::string_view view = "mnemonic";
bytes_data salt;

  salt.insert(salt.end(), view.begin(), view.end());
  if (!passphrase.empty()) {
    salt.insert(salt.end(), passphrase.begin(), passphrase.end());
  }
  return salt;
}

bool MnemonicGenerator::mnemonic_is_correct(std::string_view mnemonic) {
std::vector<uint16_t> mnemonic_indexes;

int checkSum = 0;
auto words = mnemonic | std::views::split(' ');

for(auto word_range : words) {
  std::string_view word{word_range.begin(), word_range.end()};
  int index = bip_39::getIndex(word);
  if(index < 0) return false;
  mnemonic_indexes.push_back(index);
}

checkSum = static_cast<int>(mnemonic_indexes.size() / 3);



bytes_data mnemonic_in_binary = tech_utils::toBits(mnemonic_indexes);
bytes_data check_sum_from_mnemonic (checkSum);

check_sum_from_mnemonic.assign(std::make_move_iterator(mnemonic_in_binary.end() - checkSum), std::make_move_iterator(mnemonic_in_binary.end()));

mnemonic_in_binary.erase(mnemonic_in_binary.end() - checkSum, mnemonic_in_binary.end());


bytes_data hash = hashes.sha256(tech_utils::to_bytes_from_bits(mnemonic_in_binary));
bytes_data check_sum_sha256 = crypto_utils::getCheckSum(hash[0], checkSum);

return check_sum_sha256 == check_sum_from_mnemonic;
}
