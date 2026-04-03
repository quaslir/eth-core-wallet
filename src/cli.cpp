#include "cli.hpp"
#include "utils.hpp"
namespace cli {
    void print_welcome_message(void) {
    std::cout << "============================================" << std::endl;
    std::cout << "       ETH CORE WALLET v1.0 (Dev Alpha)     " << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "[INFO] System initialized." << std::endl;
    std::cout << "[INFO] Cryptography: secp256k1 + Keccak-256" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Please select an operation:" << std::endl;
    std::cout << "  1. Generate New Wallet" << std::endl;
    std::cout << "  2. Import Wallet" << std::endl;
    std::cout << "  3. Exit" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << ">>> Option: ";
}

int prompt_entropy_selection(void) {
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

void display_mnemonic(const bytes_data& mnemonic) {
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

void confirm_liability_waiver(void) {
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

bytes_data receive_passphrase(void) {
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

 std::string request_input_mnemonic(void) {
        std::string mnemonic, passphrase;

        std::cout << "\n==================================================\n";
    std::cout << "                [ WALLET IMPORT ]                 \n";
    std::cout << "==================================================\n\n";

    std::cout << "STEP 1: ENTER MNEMONIC PHRASE\n";
    std::cout << "Input 12, 15, 18, 21, or 24 words separated by space.\n";
    std::cout << "Note: Typos will result in a different wallet address.\n";
    std::cout << "> ";

    tech_utils::clear_stdin();
    std::getline(std::cin, mnemonic);

    std::cout << "\n--------------------------------------------------\n";
    std::cout << "[ STATUS ] VALIDATING CHECKSUM...\n";
    std::cout << "--------------------------------------------------\n";

    return mnemonic;
    }

std::string request_input_optional_passphrase(void) {
std::string passprase;

std::cout << "\n--------------------------------------------------\n";
    std::cout << "STEP 2: ENTER PASSPHRASE (OPTIONAL)\n";
    std::cout << "--------------------------------------------------\n";
    std::cout << "This is an additional security layer\n";
    std::cout << "If you didn't use a passphrase before, leave it empty.\n";
    std::cout << "WARNING: If you forget it, your funds are LOST FOREVER.\n";
    std::cout << "Enter passphrase : \n";
    std::cout << "> ";

    std::getline(std::cin, passprase);

    return passprase;
}

void incorrect_mnemonic_text(void){
std::cout << "\n[ ERROR ] Invalid Mnemonic Phrase!\n";
        std::cout << "Possible reasons:\n";
        std::cout << "1. One or more words are not from the BIP-39 dictionary.\n";
        std::cout << "2. Checksum verification failed (typo in words or wrong order).\n";
        std::cout << "3. Incorrect number of words (must be 12, 15, 18, 21, or 24).\n";
        std::cout << "Please try again.\n";
        std::cout << "--------------------------------------------------\n";
}
}