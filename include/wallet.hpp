#pragma once
#include "derive.hpp"
#include "mnemonic.hpp"
#include "security.hpp"
#include "config.hpp"
#include "utils.hpp"
#include <string>
using bytes_data = std::vector<uint8_t>;
class Wallet {
private:
  bytes_data priv_key;
  bytes_data eth_address;
  bytes_data master_node;
  MnemonicGenerator mem;
  long long index = 0;

  int get_number_of_bits(void) const;

public:
  bytes_data prepare_mnemonic(Config& conf);
  void finalize_from_mnemonic(bytes_data &mnemonic, bytes_data &passphrase);
  bytes_data get_eth_address(void) const;
  bytes_data get_private_key(void) const;
  bool correct_mnemonic(std::string &mnemonic);
  void import_wallet(std::string &mnemonic, std::string &passphrase);
  long long getIndex(void) const;
};