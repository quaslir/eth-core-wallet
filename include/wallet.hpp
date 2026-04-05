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

  ~Wallet();

  bytes_data prepare_mnemonic(Config& conf);
  void finalize_from_mnemonic(bytes_data &mnemonic, bytes_data &passphrase, const std::vector<uint32_t>& path_deriv);
  const bytes_data& get_eth_address(void) const;
  const bytes_data& get_private_key(void) const;
  const bytes_data& get_master_node(void) const;

  void set_eth_address(const bytes_data& addr);
  void set_private_key(const bytes_data& private_key) ;
  void set_master_node(const bytes_data& master_n) ;
  bool correct_mnemonic(std::string &mnemonic);
  void import_wallet(std::string &mnemonic, std::string &passphrase);
  const long long& getIndex(void) const;
};