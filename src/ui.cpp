#include "ui.hpp"
#include <iostream>

void UserInterface::load(void) {
    print_welcome_message();
    make_choice_from_welcome_message();
    //print_wallet_ui();
}


void UserInterface::print_welcome_message(void) {
    std::cout << "============================================" << std::endl;
    std::cout << "       ETH CORE WALLET v1.0 (Dev Alpha)     " << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "[INFO] System initialized." << std::endl;
    std::cout << "[INFO] Cryptography: secp256k1 + Keccak-256" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Please select an operation:" << std::endl;
    std::cout << "  1. Generate New Wallet" << std::endl;
    std::cout << "  2. Import Private Key" << std::endl;
    std::cout << "  3. Derive Address from Existing Key" << std::endl;
    std::cout << "  4. Exit" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << ">>> Option: ";
}

void UserInterface::make_choice_from_welcome_message(void) {
int choice = 0;
std::cin >> choice;

while(choice != 1 && choice != 2 && choice != 3 && choice != 4) {
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
            break;
            case 3:
            break;
            case 4: 
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
   wallet.generate();

   std::cout << "PRIVATE_KEY: 0x";
  for (size_t i = 0; i < wallet.get_private_key().size(); i++) {
    printf("%02x",wallet.get_private_key()[i]);
  }

  std::cout << "\nPUBLIC_KEY: 0x";

  for (size_t i = 0; i < wallet.get_eth_address().size(); i++) {
    printf("%02x", wallet.get_eth_address()[i]);
  }
}