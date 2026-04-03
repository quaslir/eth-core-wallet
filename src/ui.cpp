#include "ui.hpp"
#include "cli.hpp"
#include <iostream>

void UserInterface::load(void) {
    cli::print_welcome_message();
    make_choice_from_welcome_message();
    print_wallet_ui();
}




void UserInterface::make_choice_from_welcome_message(void) {
int choice = 0;
std::cin >> choice;

while(choice != 1 && choice != 2 && choice != 3) {
    tech_utils::clear_stdin();
    std::cout << "Incorrect choice, please enter correct option" << std::endl;
     std::cout << ">>> Option: ";
    std::cin >> choice;
}

apply_choice_from_welcome_message(choice);
}
    void UserInterface::apply_choice_from_welcome_message(int choice) {
        switch(choice) {
            case 1:
            handle_wallet_creation();
            break;
            case 2:
            handle_wallet_import();
            break;
            case 3:
            break;
        }
}

void UserInterface::print_wallet_ui(void) const {
    std::cout << "\033[2J\033[1;1H"; 

    std::cout << "\033[1;32m" << "====================================================" << "\033[0m" << std::endl;
    std::cout << "          \033[1;37mETH CORE WALLET - SESSION ACTIVE\033[0m" << std::endl;
    std::cout << "\033[1;32m" << "====================================================" << "\033[0m" << std::endl;

    std::cout << "  [ADDRESS]  \033[1;33m"; 
    tech_utils::print_hex(wallet.get_eth_address());
    std::cout << "\033[0m" << std::endl;
    std::cout << "  [PATH]     m/44'/60'/0'/0/" << wallet.getIndex() << std::endl;
    std::cout << "  [NETWORK]  Ethereum Mainnet (Offline Mode)" << std::endl;
    std::cout << "\033[1;32m" << "----------------------------------------------------" << "\033[0m" << std::endl;

    std::cout << "  1. \033[1;37mSend Transaction\033[0m (Requires RPC)" << std::endl;
    std::cout << "  2. \033[1;37mNext Address\033[0m    (Derive Index " << wallet.getIndex() + 1 << ")" << std::endl;
    std::cout << "  3. \033[1;37mSwitch Account\033[0m  (BIP-44 Account Level)" << std::endl;
    std::cout << "  4. \033[1;37mExport Key\033[0m      (Show Private Hex)" << std::endl;
    std::cout << "  5. \033[1;31mLock & Exit\033[0m     (Wipe Memory)" << std::endl;

    std::cout << "\033[1;32m" << "====================================================" << "\033[0m" << std::endl;
    std::cout << ">>> Select action: ";
}

void UserInterface::handle_wallet_creation(void) {
    int strength = cli::prompt_entropy_selection();
   bytes_data mnemonic = wallet.prepare_mnemonic(strength);
   cli::display_mnemonic(mnemonic);
   cli::confirm_liability_waiver();
   bytes_data passphrase = cli::receive_passphrase();
   wallet.finalize_from_mnemonic(mnemonic, passphrase);
}


void UserInterface::handle_wallet_import(void) {
std::string mnemonic = cli::request_input_mnemonic();

while(!wallet.correct_mnemonic(mnemonic)) {
cli::incorrect_mnemonic_text();
mnemonic = cli::request_input_mnemonic();
}

std::string passphrase = cli::request_input_optional_passphrase();
wallet.import_wallet(mnemonic, passphrase);
}
