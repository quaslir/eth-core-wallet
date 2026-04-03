#include <string>
#include "wallet.hpp"
#include "config.hpp"

class UserInterface {
    public:
    void load(void);
    private:
    Wallet wallet;
    Config config;
    void handle_wallet_creation(void);
    void handle_wallet_import(void);
    void make_choice_from_welcome_message(void);
    void apply_choice_from_welcome_message(int choice); 
    bool handle_seed_generation_config(void);
};