#include "mnemonic.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>
MnemonicGenerator::MnemonicGenerator() {}

void MnemonicGenerator::createKey(std::vector<uint8_t> && randNumber, uint8_t checksum) {
int count = 0;
uint16_t index = 0;
for(size_t i = 0;i < randNumber.size(); i++) {
    bool lastByte = i == randNumber.size() - 1;
    int end = lastByte ? 7 - (checksum - 1) : 0;
   for(int start = 7; start >= end; start--) {
        bool bit = (randNumber[i] >> start) & 1;
        index = (index << 1) | bit;
        count++;
        if(count == 11) {
            key.push_back(index);
            index = 0;
            count = 0;
        }
    }
}


for(const auto &c :key) {
    std::cout << c << " ";
}
}

void MnemonicGenerator::generateSeedPhrase(uint16_t bits) {
size_t bytes = bits / 8;
size_t checkSum = bits / 32;
std::vector<uint8_t> randNumber = genNumber(bytes);
std::vector<uint8_t> hash = hashes.sha256(randNumber);
randNumber.push_back(hash[0]);
createKey(std::move(randNumber), checkSum);
}

std::vector<uint8_t> MnemonicGenerator::genNumber(size_t bytes) const {
std::vector<uint8_t> buf(bytes);

if(RAND_bytes(buf.data(), static_cast<int>(bytes)) != 1) {
    throw std::runtime_error("OpenSSL: Failed to generate cryptographically strong random bytes.");
}

return buf;
}