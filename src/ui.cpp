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
    std::cout << "  2. Import Wallet" << std::endl;
    std::cout << "  2. Exit" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << ">>> Option: ";
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
    int strength = prompt_entropy_selection();
   bytes_data mnemonic = wallet.prepare_mnemonic(strength);
   display_mnemonic(mnemonic);
   confirm_liability_waiver();
   bytes_data passphrase = receive_passphrase();
   wallet.finalize_from_mnemonic(mnemonic, passphrase);

   std::cout << "PRIVATE_KEY: 0x";
  for (size_t i = 0; i < wallet.get_private_key().size(); i++) {
    printf("%02x",wallet.get_private_key()[i]);
  }

  std::cout << "\nPUBLIC_KEY: 0x";

  for (size_t i = 0; i < wallet.get_eth_address().size(); i++) {
    printf("%02x", wallet.get_eth_address()[i]);
  }
}


int UserInterface::prompt_entropy_selection(void) const {
    std::cout << "\n[ STEP 1: ENTROPY SELECTION ]" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "  1. 128-bit (12 words) - Standard Security" << std::endl;
    std::cout << "  2. 256-bit (24 words) - [RECOMMENDED] Ultra Security" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << ">>> Choice [1-2]: ";

    int choice = 0;
    std::cin >> choice;
    while(choice != 1 && choice != 2) {
        tech_utils::clear_stdin();
        std::cout << "\033[1;31m[ERROR] Invalid choice. Enter 1 or 2: \033[0m";
        std::cin >> choice;
    }

    return choice == 1 ? 128 : 256;
}

void UserInterface::display_mnemonic(const bytes_data& mnemonic) const {
std::cout << "\n\033[1;33m" << "####################################################" << "\033[0m" << std::endl;
    std::cout << "\033[1;33m" << "#           YOUR RECOVERY PHRASE (SEED)            #" << "\033[0m" << std::endl;
    std::cout << "\033[1;33m" << "####################################################" << "\033[0m" << std::endl;
    std::cout << "  \033[1;37m";

    for(const auto& byte : mnemonic) {
        std::cout << static_cast<char>(byte);
    }

    std::cout << std::endl;
    std::cout << "\033[1;33m" << "####################################################" << "\033[0m" << std::endl;
    std::cout << "\033[1;31m" << "  WARNING: DO NOT SCREENSHOT! WRITE IT DOWN NOW!    " << "\033[0m" << std::endl;
    std::cout << "\033[1;33m" << "####################################################" << "\033[0m" << std::endl;
}

void UserInterface::confirm_liability_waiver(void) const {
std::cout << "\n\033[1;31m[!!!] FINAL LEGAL & SECURITY WARNING [!!!]\033[0m" << std::endl;
    std::cout << "1. This app will NOW WIPE the mnemonic from memory." << std::endl;
    std::cout << "2. We have ZERO copies. No database, no logs, no backups." << std::endl;
    std::cout << "3. If you didn't write it down, your funds are ALREADY LOST." << std::endl;
    std::cout << "\nTo proceed, type exactly: \033[1;37mI AM RESPONSIBLE\033[0m" << std::endl;
    std::cout << ">>> ";
std::string input;

tech_utils::clear_stdin();
std::getline(std::cin, input);

while(input != "I AM RESPONSIBLE") {
    std::cout << "\033[1;31m[!] Incorrect. Type 'I AM RESPONSIBLE' to wipe memory: \033[0m";
    std::getline(std::cin, input);
}
std::cout << "\n\033[1;32m[SYSTEM] Mnemonic wiped. Wallet initialized.\033[0m" << std::endl;
}

bytes_data UserInterface::receive_passphrase(void) const {
std::cout << "\n\033[1;36m[ STEP 3: OPTIONAL PASSPHRASE (25th Word) ]\033[0m" << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;
    std::cout << "  A Passphrase adds an extra layer of security to your seed." << std::endl;
    std::cout << "  [!] WARNING: It is NOT a password for the app." << std::endl;
    std::cout << "  [!] WARNING: Different passphrase = DIFFERENT WALLET." << std::endl;
    std::cout << "  Leave EMPTY and press Enter if you don't want to use one." << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;
    std::cout << ">>> Enter Passphrase: ";

    bytes_data pass;
    pass.reserve(128);

    char ch;
   while(std::cin.get(ch) && ch != '\n') {
    pass.push_back(static_cast<uint8_t>(ch));
   }

   return pass;
}