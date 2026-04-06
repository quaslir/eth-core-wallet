#include "mnemonic.hpp"
#include "bip39.hpp"
#include "config.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <ranges>
MnemonicGenerator::MnemonicGenerator() {}

bytes_data MnemonicGenerator::createMnemonic(bytes_data &randNumber,
                                             bytes_data &checkSum) {
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

bytes_data MnemonicGenerator::generateMnemonic(Config& conf) {
  size_t bytes = conf.bit_length / 8;
  int checkSumBits = conf.bit_length / 32;
  bytes_data randNumber = crypto_utils::genNumber(bytes);
  if(!conf.extra_entropy.empty()) {
    randNumber = handle_extra_entropy_from_user(randNumber, conf.extra_entropy, conf.bit_length);
  }
  bytes_data hash = hashes.sha256(randNumber);

  bytes_data checksum = crypto_utils::getCheckSum(hash[0], checkSumBits);
  OPENSSL_cleanse(hash.data(), hash.size());
  bytes_data mnemonic = createMnemonic(randNumber, checksum);
  return mnemonic;
}

bytes_data MnemonicGenerator::generateSeed(bytes_data &mnemonic,
                                           bytes_data &passphrase) {
  bytes_data salt = createSalt(passphrase);

  bytes_data masterseed = crypto_utils::PBKDF2_HMAC_SHA512(mnemonic, salt, 2048);
  OPENSSL_cleanse(mnemonic.data(), mnemonic.size());
  OPENSSL_cleanse(passphrase.data(), passphrase.size());
  OPENSSL_cleanse(salt.data(), salt.size());  
  return masterseed;
}

bytes_data MnemonicGenerator::createSalt(bytes_data &passphrase) {
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

  for (auto word_range : words) {
    std::string_view word{word_range.begin(), word_range.end()};
    int index = bip_39::getIndex(word);
    if (index < 0)
      return false;
    mnemonic_indexes.push_back(index);
  }

  checkSum = static_cast<int>(mnemonic_indexes.size() / 3);

  bytes_data mnemonic_in_binary = tech_utils::toBits(mnemonic_indexes);
  OPENSSL_cleanse(mnemonic_indexes.data(),
                  mnemonic_indexes.size() * sizeof(uint16_t));
  bytes_data check_sum_from_mnemonic(checkSum);

  check_sum_from_mnemonic.assign(
  mnemonic_in_binary.end() - checkSum, mnemonic_in_binary.end());


  mnemonic_in_binary.erase(mnemonic_in_binary.end() - checkSum,
                           mnemonic_in_binary.end());

  bytes_data hash =
      hashes.sha256(tech_utils::to_bytes_from_bits(mnemonic_in_binary));
  OPENSSL_cleanse(mnemonic_in_binary.data(), mnemonic_in_binary.size());

  bytes_data check_sum_sha256 = crypto_utils::getCheckSum(hash[0], checkSum);

  bool result = check_sum_sha256 == check_sum_from_mnemonic;

  OPENSSL_cleanse(check_sum_from_mnemonic.data(),
                  check_sum_from_mnemonic.size());
  OPENSSL_cleanse(check_sum_sha256.data(), check_sum_sha256.size());

  return result;
}

bytes_data MnemonicGenerator::handle_extra_entropy_from_user(bytes_data& entropy, bytes_data& extra_entropy, int target_bits) {
entropy.insert(entropy.end(), extra_entropy.begin(), extra_entropy.end());


bytes_data new_entropy = hashes.sha256(entropy);

OPENSSL_cleanse(extra_entropy.data(), extra_entropy.size());
OPENSSL_cleanse(entropy.data(), entropy.size());

int target_bytes = target_bits / 8;

bytes_data final_entropy(new_entropy.begin(), new_entropy.begin() + target_bytes);

OPENSSL_cleanse(new_entropy.data(), new_entropy.size());

return final_entropy;
}