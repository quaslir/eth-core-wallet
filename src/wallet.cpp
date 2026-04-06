#include "wallet.hpp"
#include "config.hpp"
#include <iostream>

Wallet::~Wallet() {
  OPENSSL_cleanse(priv_key.data(), priv_key.size());
  OPENSSL_cleanse(eth_address.data(), eth_address.size());
  OPENSSL_cleanse(master_node.data(), master_node.size());
}


int Wallet::get_number_of_bits(void) const { return 0; }

bytes_data Wallet::prepare_mnemonic(Config& conf) {
  return mem.generateMnemonic(conf);
}

void Wallet::derive(const std::vector<uint32_t>& path_deriv) {
    KEY_PAIR keys;
  crypto_utils::split_key(master_node, keys.parent_key, keys.chain_key);
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
                                    bytes_data &passphrase, const std::vector<uint32_t>& path_deriv) {
  bytes_data seed = mem.generateSeed(mnemonic, passphrase);

  OPENSSL_cleanse(passphrase.data(), passphrase.size());
  OPENSSL_cleanse(mnemonic.data(), mnemonic.size());

     std::string_view key = "Bitcoin seed";
  master_node = crypto_utils::HMAC_SHA512(key, seed);     
    OPENSSL_cleanse(seed.data(), seed.size());                              
            
  derive(path_deriv);
  security_manager::first_time_save(*this);
}

const bytes_data& Wallet::get_eth_address(void) const { return this->eth_address; }
const bytes_data& Wallet::get_private_key(void) const { return this->priv_key; }
const long long &Wallet::getIndex(void) const { return this->index; }
const bytes_data& Wallet::get_master_node(void) const {return this->master_node;}

 void Wallet::set_eth_address(const bytes_data& addr) {
 this->eth_address = addr;
 }
  void Wallet::set_private_key(const bytes_data& private_key)  {
    this->priv_key = private_key;
  }
  void Wallet::set_master_node(const bytes_data& master_n)  {
    this->master_node = master_n;
  }


bool Wallet::correct_mnemonic(std::string &mnemonic) {
  if (mnemonic.empty())
    return false;
  return mem.mnemonic_is_correct(mnemonic);
}

void Wallet::import_wallet(std::string &mnemonic, std::string &passphrase) {
  bytes_data bytes_mnemonic(mnemonic.begin(), mnemonic.end());
  bytes_data bytes_passphrase(passphrase.begin(), passphrase.end());

  finalize_from_mnemonic(bytes_mnemonic, bytes_passphrase, crypto_utils::path_deriv);
  OPENSSL_cleanse(mnemonic.data(), mnemonic.size());
  OPENSSL_cleanse(passphrase.data(), passphrase.size());
}