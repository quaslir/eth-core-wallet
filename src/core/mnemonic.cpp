#include "core/mnemonic.hpp"
#include "api/json.hpp"
#include "config/config.hpp"
#include "core/bip39.hpp"
#include "core/secure_bytes_data.hpp"
#include "utils/tech_utils.hpp"
#include <ranges>
MnemonicGenerator::MnemonicGenerator() {}

secure_string MnemonicGenerator::createMnemonic(const bytes_data &randNumber,
                                                const bytes_data &checkSum) {
  bytes_data seed;
  seed.reserve(randNumber.size() * 8 + checkSum.size());

  for (const auto &byte : randNumber) {
    for (int i = 7; i >= 0; i--) {
      seed.push_back((byte >> i) & 1);
    }
  }
  seed.insert(seed.end(), checkSum.begin(), checkSum.end());

  std::vector<uint16_t> indexes;

  for (size_t i = 0; i < seed.size(); i += 11) {
    uint16_t index = 0;

    for (int j = 0; j < 11; j++) {
      index = (index << 1) | seed[i + j];
    }

    indexes.push_back(index);
  }
  secure_string mnemonic;

  for (size_t i = 0; i < indexes.size(); i++) {
    std::string_view word = bip_39::bip_39_words[indexes[i]];
    mnemonic.insert(mnemonic.end(), word.begin(), word.end());
    if (i + 1 < indexes.size()) {
      mnemonic.append(" ");
    }
  }
  return mnemonic;
}

secure_string MnemonicGenerator::generateMnemonic(const Config &conf) const {
  size_t bytes = conf.bit_length / 8;
  int checkSumBits = conf.bit_length / 32;
  bytes_data randNumber = crypto_utils::gen_number(bytes);
  if (!conf.extra_entropy.empty()) {
    randNumber = handle_extra_entropy_from_user(randNumber, conf.extra_entropy,
                                                conf.bit_length);
  }
  bytes_data hash = hashes.sha256(randNumber);

  bytes_data checksum = crypto_utils::getCheckSum(hash[0], checkSumBits);
  return createMnemonic(randNumber, checksum);
}

bytes_data
MnemonicGenerator::generateSeed(const secure_string &mnemonic,
                                const secure_string &passphrase) const {
  bytes_data salt = createSalt(passphrase);

  bytes_data masterseed =
      crypto_utils::PBKDF2_HMAC_SHA512(mnemonic, salt, 2048);

  return masterseed;
}

bytes_data MnemonicGenerator::createSalt(const secure_string &passphrase) {
  std::string_view view = "mnemonic";
  bytes_data salt;

  salt.insert(salt.end(), view.begin(), view.end());
  if (!passphrase.empty()) {
    salt.insert(salt.end(), passphrase.begin(), passphrase.end());
  }
  return salt;
}

bool MnemonicGenerator::mnemonic_is_correct(
    const secure_string &mnemonic) const {
  if (!tech_utils::contains_only_lowercase(mnemonic))
    return false;
  std::vector<uint16_t> mnemonic_indexes;

  int checkSum = 0;
  auto words = mnemonic | std::views::split(' ') |
               std::views::filter([](auto &&r) { return !r.empty(); });

  for (auto word_range : words) {
    std::string_view word{word_range.begin(), word_range.end()};
    if (word.empty())
      return false;
    int index = bip_39::getIndex(word);

    if (index < 0)
      return false;
    mnemonic_indexes.push_back(index);
  }
  if (mnemonic_indexes.size() != 12 && mnemonic_indexes.size() != 15 &&
      mnemonic_indexes.size() != 18 && mnemonic_indexes.size() != 21 &&
      mnemonic_indexes.size() != 24)
    return false;

  checkSum = static_cast<int>(mnemonic_indexes.size() / 3);

  bytes_data mnemonic_in_binary = tech_utils::to_bits(mnemonic_indexes);

  bytes_data check_sum_from_mnemonic(checkSum);

  check_sum_from_mnemonic.assign(mnemonic_in_binary.end() - checkSum,
                                 mnemonic_in_binary.end());

  mnemonic_in_binary.erase(mnemonic_in_binary.end() - checkSum,
                           mnemonic_in_binary.end());

  bytes_data hash =
      hashes.sha256(tech_utils::to_bytes_from_bits(mnemonic_in_binary));

  bytes_data check_sum_sha256 = crypto_utils::getCheckSum(hash[0], checkSum);

  bool result = check_sum_sha256 == check_sum_from_mnemonic;

  return result;
}

bytes_data MnemonicGenerator::handle_extra_entropy_from_user(
    const bytes_data &entropy, const bytes_data &extra_entropy,
    int target_bits) const {

  bytes_data combined;
  combined.reserve(entropy.size() + extra_entropy.size());
  combined.insert(combined.end(), entropy.begin(), entropy.end());
  combined.insert(combined.end(), extra_entropy.begin(), extra_entropy.end());

  bytes_data new_entropy = hashes.sha256(combined);

  int target_bytes = target_bits / 8;

  bytes_data final_entropy(new_entropy.begin(),
                           new_entropy.begin() + target_bytes);

  return final_entropy;
}

secure_string
MnemonicGenerator::__generateMnemonic(const bytes_data &entropy) const {
  bytes_data hash = hashes.sha256(entropy);
  int checkSumBits = entropy.size() * 8 / 32;
  bytes_data checksum = crypto_utils::getCheckSum(hash[0], checkSumBits);

  return createMnemonic(entropy, checksum);
}