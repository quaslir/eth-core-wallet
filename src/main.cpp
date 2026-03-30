#include "utils.hpp"
#include <iomanip>
#include <iostream>
#include <vector>
int main(void) {
  crypto_utils::Hashes hashes;
  std::vector<uint8_t> msg = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  std::vector<uint8_t> hash = hashes.sha256(msg);

  for (const auto &c : hash) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(c);
  }

  std::cout << std::dec << std::endl;
  return 0;
}