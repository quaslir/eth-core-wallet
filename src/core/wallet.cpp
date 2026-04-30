#include "core/wallet.hpp"
#include "config/config.hpp"
#include "core/security.hpp"
#include "utils/tech_utils.hpp"
#include <iostream>
#include <string_view>

Wallet::~Wallet() {
// handle
}

int Wallet::get_number_of_bits(void) const { return 0; }

bytes_data Wallet::prepare_mnemonic(const Config &conf) const {
  return mem.generateMnemonic(conf);
}

void Wallet::sync_derive_path(std::vector<uint32_t> &derive_path) const {
  derive_path.back() = this->index; //!
}
bool Wallet::update_index() const {

  if (!security_manager::update(*this))
    return false;
  return true;
}
void Wallet::save(const bytes_data &password, const std::string &filename) const {
  security_manager::first_time_save(*this, password, filename);
}
void Wallet::derive(const std::vector<uint32_t> &path_deriv) {
  KEY_PAIR keys;
  crypto_utils::split_key_64(master_node, keys.parent_key, keys.chain_key);
  Key_Derive devk;
  for (size_t i = 0; i < path_deriv.size(); i++) {
    devk.derive_child(keys, path_deriv[i]);
  }

  eth_address = devk.generate_address(keys.parent_key);

  priv_key = keys.parent_key;
}

void Wallet::finalize_from_mnemonic(const bytes_data &mnemonic,
                                    const bytes_data &passphrase,
                                    const std::vector<uint32_t> &path_deriv) {
  bytes_data seed = mem.generateSeed(mnemonic, passphrase);

  std::string_view key = "Bitcoin seed";
  master_node = crypto_utils::HMAC_SHA512(key, seed);

  derive(path_deriv);
}

std::string Wallet::get_eth_address(void) const { return this->eth_address; }
const bytes_data &Wallet::get_private_key(void) const { return this->priv_key; }

std::string Wallet::__get_private_key_hex(
    void) const { // Used only in tests, this method is insecure and dangerous
  return "0x" + tech_utils::to_hex(priv_key);
}
const long long &Wallet::getIndex(void) const { return this->index; }
const bytes_data &Wallet::get_master_node(void) const {
  return this->master_node;
}

double Wallet::get_balance(void) const { return this->current_balance; }
void Wallet::set_balance(double balance) { this->current_balance = balance; }

bool Wallet::derive_next(void) {
  if (index >= 0x7FFFFFF)
    return false;

  try {
    std::vector<uint32_t> next_path_deriv =
        crypto_utils::change_derive_path(index + 1);
    derive(next_path_deriv);
    index++;
  } catch (const std::exception &err) {
    std::cerr << "[Wallet Error] Derivation failed: " << err.what()
              << std::endl;
    return false;
  }

  return true;
}

bool Wallet::derive_prev(void) {
  if (index <= 0)
    return false;

  try {
    std::vector<uint32_t> prev_path_deriv =
        crypto_utils::change_derive_path(index - 1);

    derive(prev_path_deriv);
    index--;
  } catch (const std::exception &err) {
    std::cerr << "[Wallet Error] Derivation failed: " << err.what()
              << std::endl;
    return false;
  }

  return true;
}

void Wallet::set_private_key(const bytes_data &private_key) {
  this->priv_key = private_key;
}
void Wallet::set_master_node(const bytes_data &master_n) {
  this->master_node = master_n;
}
void Wallet::set_index(const int i) { this->index = i; }

bool Wallet::correct_mnemonic(std::string_view mnemonic) const {
  if (mnemonic.empty())
    return false;
  return mem.mnemonic_is_correct(mnemonic);
}

void Wallet::import_wallet(const bytes_data &mnemonic, const bytes_data &passphrase) {
  finalize_from_mnemonic(mnemonic, passphrase, crypto_utils::path_deriv);
}

bool Wallet::is_loaded(void) const { return !eth_address.empty(); }
