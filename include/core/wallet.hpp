#pragma once
#include "config/config.hpp"
#include "core/secure_bytes_data.hpp"
#include "derive.hpp"
#include "mnemonic.hpp"
#include "security.hpp"
#include <string>

class Wallet {
private:
  bytes_data priv_key;
  secure_string eth_address;
  bytes_data master_node;
  double current_balance;
  MnemonicGenerator mem;
  long long index = 0;
  int get_number_of_bits(void) const;

public:
  ~Wallet();
  secure_string prepare_mnemonic(const Config &conf) const;
  void finalize_from_mnemonic(const secure_string &mnemonic,
                              const secure_string &passphrase,
                              const std::vector<uint32_t> &path_deriv);
  secure_string get_eth_address(void) const;
  const bytes_data &get_private_key(void) const;
  const bytes_data &get_master_node(void) const;
  secure_string __get_private_key_hex(void) const;

  double get_balance(void) const;
  void sync_derive_path(std::vector<uint32_t> &derive_path) const;

  void set_private_key(const bytes_data &private_key);
  void set_master_node(const bytes_data &master_n);
  void set_balance(double balance);
  void set_index(const int i);
  bool derive_next(void);
  bool derive_prev(void);
  void derive(const std::vector<uint32_t> &path_deriv);
  bool correct_mnemonic(const secure_string &mnemonic) const;
  void import_wallet(const secure_string &mnemonic,
                     const secure_string &passphrase);
  const long long &getIndex(void) const;

  bool update_index(void) const;
  void save(const secure_string &password,
            const std::string &filename = "session.json") const;
  bool is_loaded(void) const;
};
