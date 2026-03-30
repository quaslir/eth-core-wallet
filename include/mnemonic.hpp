#include <stdint.h>
#include <vector>
#include <utils.hpp>
#include <openssl/rand.h>

class MnemonicGenerator {
private:
std::vector<uint16_t> key;
crypto_utils::Hashes hashes;

std::vector<uint8_t> genNumber(size_t bytes) const;
void createKey(std::vector<uint8_t> && randNumber, uint8_t checkSum);
public:
  MnemonicGenerator();
  void generateSeedPhrase(uint16_t bits);
};