#include <string>
#include "wallet.hpp"


class UserInterface {
    public:

    void load(void);
    private:
    Wallet wallet;
    void handle_wallet_creation(void);
    static void print_welcome_message(void);
    void make_choice_from_welcome_message(void);
    void apply_choice_from_welcome_message(int choice);

};