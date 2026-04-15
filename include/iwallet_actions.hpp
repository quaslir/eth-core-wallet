#pragma once



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
    virtual void handle_config_menu(int choice) = 0;
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

};
