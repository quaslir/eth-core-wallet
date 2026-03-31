#include "mnemonic.hpp"
#include <iomanip>
#include <iostream>
#include <vector>
int main(void) {
  MnemonicGenerator mem;
  mem.generateSeedPhrase(256);
  return 0;
}