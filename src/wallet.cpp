#include "wallet.hpp"
#include "config.hpp"
#include "security.hpp"
#include <iostream>
#include <mutex>

Wallet::~Wallet() {
  OPENSSL_cleanse(priv_key.data(), priv_key.size());
  OPENSSL_cleanse(eth_address.data(), eth_address.size());
  OPENSSL_cleanse(master_node.data(), master_node.size());
}

int Wallet::get_number_of_bits(void) const { return 0; }

bytes_data Wallet::prepare_mnemonic(Config &conf) const {
  return mem.generateMnemonic(conf);
}

void Wallet::sync_derive_path(std::vector<uint32_t> &derive_path) const {
  derive_path.back() = this->index;
}
bool Wallet::update_index() const {

  if (!security_manager::update(*this))
    return false;
  return true;
}
void Wallet::save(bytes_data &password, const std::string &filename) const {
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
  OPENSSL_cleanse(keys.parent_key.data(), keys.parent_key.size());
  OPENSSL_cleanse(keys.chain_key.data(), keys.chain_key.size());
}

void Wallet::finalize_from_mnemonic(bytes_data &mnemonic,
                                    bytes_data &passphrase,
                                    const std::vector<uint32_t> &path_deriv) {
  bytes_data seed = mem.generateSeed(mnemonic, passphrase);

  OPENSSL_cleanse(passphrase.data(), passphrase.size());
  // OPENSSL_cleanse(mnemonic.data(), mnemonic.size());

  std::string_view key = "Bitcoin seed";
  master_node = crypto_utils::HMAC_SHA512(key, seed);
  OPENSSL_cleanse(seed.data(), seed.size());

  derive(path_deriv);
}

bytes_data Wallet::get_eth_address(void) const {
  std::lock_guard<std::mutex> lock(wallet_mutex);
  return this->eth_address;
}
const bytes_data &Wallet::get_private_key(void) const { return this->priv_key; }
const long long &Wallet::getIndex(void) const { return this->index; }
const bytes_data &Wallet::get_master_node(void) const {
  return this->master_node;
}

std::string Wallet::get_balance(void) const { return this->current_balance; }
void Wallet::set_balance(const std::string &balance) {
  this->current_balance = balance;
}

bool Wallet::derive_next(void) {
  if (index >= 0x7FFFFFF)
    return false;

  try {
    std::vector<uint32_t> next_path_deriv =
        crypto_utils::change_derive_path(index + 1);
    OPENSSL_cleanse(priv_key.data(), priv_key.size());
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
    OPENSSL_cleanse(priv_key.data(), priv_key.size());
    derive(prev_path_deriv);
    index--;
  } catch (const std::exception &err) {
    std::cerr << "[Wallet Error] Derivation failed: " << err.what()
              << std::endl;
    return false;
  }

  return true;
}

void Wallet::set_eth_address(const bytes_data &addr) {
  this->eth_address = addr;
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

void Wallet::import_wallet(bytes_data &mnemonic, bytes_data &passphrase) {
  finalize_from_mnemonic(mnemonic, passphrase, crypto_utils::path_deriv);
  OPENSSL_cleanse(mnemonic.data(), mnemonic.size());
  OPENSSL_cleanse(passphrase.data(), passphrase.size());
}

bool Wallet::is_loaded(void) const { return !eth_address.empty(); }
