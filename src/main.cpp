#include "mnemonic.hpp"
#include <iomanip>
#include <iostream>
#include <vector>
int main(void) {
  MnemonicGenerator mem;
  std::vector<uint8_t> seed = mem.generateSeed(256);

  return 0;
}