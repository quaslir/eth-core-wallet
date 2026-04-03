#include <string>
#include "wallet.hpp"


class UserInterface {
    public:
    void load(void);
    private:
    Wallet wallet;
    void handle_wallet_creation(void);
    void handle_wallet_import(void);
    std::string request_input_mnemonic(void);
    static void print_welcome_message(void);
    void make_choice_from_welcome_message(void);
    void apply_choice_from_welcome_message(int choice);
    void print_wallet_ui(void) const;
    int prompt_entropy_selection(void) const;
    void display_mnemonic(const bytes_data& mnemonic) const;
    void confirm_liability_waiver(void) const;
    bytes_data receive_passphrase(void) const;
};