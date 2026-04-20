#pragma once
#include "config.hpp"
#include "derive.hpp"
#include "mnemonic.hpp"
#include "security.hpp"
#include "tech_utils.hpp"
#include <string>

using bytes_data = std::vector<uint8_t>;
class Wallet {
private:
  bytes_data priv_key;
  bytes_data eth_address;
  bytes_data master_node;
  std::string current_balance;
  MnemonicGenerator mem;
  long long index = 0;
  mutable std::mutex wallet_mutex;
  int get_number_of_bits(void) const;

public:
  ~Wallet();
  bytes_data prepare_mnemonic(Config &conf) const;
  void finalize_from_mnemonic(bytes_data &mnemonic, bytes_data &passphrase,
                              const std::vector<uint32_t> &path_deriv);
  bytes_data get_eth_address(void) const;
  const bytes_data &get_private_key(void) const;
  const bytes_data &get_master_node(void) const;

  std::string get_balance(void) const;
  void sync_derive_path(std::vector<uint32_t> &derive_path) const;
  void set_eth_address(const bytes_data &addr);
  void set_private_key(const bytes_data &private_key);
  void set_master_node(const bytes_data &master_n);
  void set_balance(const std::string &balance);
  void set_index(const int i);
  bool derive_next(void);
  bool derive_prev(void);
  void derive(const std::vector<uint32_t> &path_deriv);
  bool correct_mnemonic(std::string_view mnemonic) const;
  void import_wallet(bytes_data &mnemonic, bytes_data &passphrase);
  const long long &getIndex(void) const;

  bool update_index(void) const;
  void save(bytes_data &password,
            const std::string &filename = "session.json") const;
  bool is_loaded(void) const;
};
