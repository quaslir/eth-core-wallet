#pragma once



#include "async_manager.hpp"
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

using bytes_data = std::vector<uint8_t>;

class Wallet;
struct Config;
class IWalletActions {
    public:
    virtual ~IWalletActions() = default;
    virtual std::string get_mnemonic(void) = 0;
    virtual const Config& get_config(void) = 0;
    virtual const Wallet& get_wallet(void)  = 0;
    virtual void on_main_menu(int choice) = 0;
    virtual void set_password_for_wallet(bytes_data& password) = 0;
    virtual bytes_data get_password_for_wallet(void) = 0;
    virtual bool check_mnemonic(std::string_view mnemonic) = 0;
    virtual void set_mnemonic(std::string_view mnemonic) = 0;
     virtual void set_passphrase(std::string_view passphrase) = 0;
     virtual void import_wallet(void) = 0;
     virtual bool check_password(bytes_data& password) = 0;
     virtual void load_wallet(void) = 0;
     virtual void save_wallet(void) = 0;
     virtual void change_bit_length(int new_bit_length) = 0;
     virtual void set_extra_entropy(std::string_view entropy) = 0;
     virtual void add_passphrase(const bytes_data& passphrase) = 0;
     virtual void change_derivation_path(std::string_view derive_path) = 0;
     virtual void create_wallet(void) = 0;
     virtual void update_balance(void) = 0;
     virtual void copy_address(void) = 0;
     virtual void copy_private_key(void) = 0;
     virtual void apply_choice_from_wallet_ui(int choice) = 0;
     virtual const bytes_data& get_private_key(void) =0;
     virtual std::vector<TransactionRecord>get_transactions_history(void) = 0;
     virtual void request_transactions_data(void) = 0;
     virtual void update_transactions_data(void) = 0;
     virtual const std::string& get_current_network(void)  = 0;
};
