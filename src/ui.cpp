#include "ui.hpp"
#include <iostream>

void UserInterface::load(void) {
    print_welcome_message();
    make_choice_from_welcome_message();
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