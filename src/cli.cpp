#include "cli.hpp"
#include "config.hpp"
#include "utils.hpp"
#include <iomanip>
namespace cli {
void print_welcome_message(std::string_view error_msg) {
  std::cout << "\033[2J\033[1;1H";
  std::cout << "============================================" << std::endl;
  std::cout << "       ETH CORE WALLET v1.0 (Dev Alpha)     " << std::endl;
  std::cout << "============================================" << std::endl;
  if(!error_msg.empty()) {
    std::cout << "\033[1;31m[!] " << error_msg << "\033[0m\n";
    std::cout << "--------------------------------------------\n";
  }
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

int make_choice_from_welcome_message(void) {
  std::string choice = tech_utils::read_stdin();
  while (choice != "1" && choice != "2" && choice != "3") {
    std::cout << "[!] Invalid input. Please enter a number between 1 and 3.\n";
    std::cout << ">>> Option: ";
   choice = tech_utils::read_stdin();
  }
  return static_cast<int>(choice.front() - '0');
}

void display_mnemonic(const bytes_data &mnemonic) {
  std::cout << "\033[2J\033[1;1H";
  std::cout << "\n\033[1;33m"
            << "####################################################"
            << "\033[0m" << std::endl;
  std::cout << "\033[1;33m"
            << "#           YOUR RECOVERY PHRASE (SEED)            #"
            << "\033[0m" << std::endl;
  std::cout << "\033[1;33m"
            << "####################################################"
            << "\033[0m" << std::endl;
  std::cout << "  \033[1;37m";

  for (const auto &byte : mnemonic) {
    std::cout << static_cast<char>(byte);
  }

  std::cout << std::endl;
  std::cout << "\033[1;33m"
            << "####################################################"
            << "\033[0m" << std::endl;
  std::cout << "\033[1;31m"
            << "  WARNING: DO NOT SCREENSHOT! WRITE IT DOWN NOW!    "
            << "\033[0m" << std::endl;
  std::cout << "\033[1;33m"
            << "####################################################"
            << "\033[0m" << std::endl;
}

void confirm_liability_waiver(std::string_view error_msg) {
  std::cout << "\n\033[1;31m[!!!] FINAL LEGAL & SECURITY WARNING [!!!]\033[0m"
            << std::endl;
  std::cout << "1. This app will NOW WIPE the mnemonic from memory."
            << std::endl;
  std::cout << "2. We have ZERO copies. No database, no logs, no backups."
            << std::endl;
  std::cout << "3. If you didn't write it down, your funds are ALREADY LOST."
            << std::endl;
  std::cout << "\nTo proceed, type exactly: \033[1;37mI AM RESPONSIBLE\033[0m"
            << std::endl;
          if(!error_msg.empty()) {
    std::cout << "\033[1;31m[!] " << error_msg << "\033[0m\n";
    std::cout << "--------------------------------------------\n";
  }
  std::cout << ">>> ";
  std::string input = tech_utils::read_stdin();
  
  

  while (input != "I AM RESPONSIBLE") {
    std::cout << "\033[1;31m[!] Incorrect. Type 'I AM RESPONSIBLE' to wipe "
                 "memory: \033[0m";
    input = tech_utils::read_stdin();
  }
}

std::string request_input_mnemonic(void) {
  std::string mnemonic, passphrase;
  std::cout << "\033[2J\033[1;1H";
  std::cout << "\n==================================================\n";
  std::cout << "                [ WALLET IMPORT ]                 \n";
  std::cout << "==================================================\n\n";

  std::cout << "STEP 1: ENTER MNEMONIC PHRASE\n";
  std::cout << "Input 12, 15, 18, 21, or 24 words separated by space.\n";
  std::cout << "Note: Typos will result in a different wallet address.\n";
  std::cout << "> ";


  mnemonic = tech_utils::read_stdin();

  std::cout << "\n--------------------------------------------------\n";
  std::cout << "[ STATUS ] VALIDATING CHECKSUM...\n";
  std::cout << "--------------------------------------------------\n";

  return mnemonic;
}

std::string request_input_optional_passphrase(void) {
  std::string passprase;
  std::cout << "\033[2J\033[1;1H";
  std::cout << "\n--------------------------------------------------\n";
  std::cout << "STEP 2: ENTER PASSPHRASE (OPTIONAL)\n";
  std::cout << "--------------------------------------------------\n";
  std::cout << "This is an additional security layer\n";
  std::cout << "If you didn't use a passphrase before, leave it empty.\n";
  std::cout << "WARNING: If you forget it, your funds are LOST FOREVER.\n";
  std::cout << "Enter passphrase : \n";
  std::cout << "> ";

  passprase = tech_utils::read_stdin();

  return passprase;
}

void incorrect_mnemonic_text(void) {
  std::cout << "\033[2J\033[1;1H";
  std::cout << "\n[ ERROR ] Invalid Mnemonic Phrase!\n";
  std::cout << "Possible reasons:\n";
  std::cout << "1. One or more words are not from the BIP-39 dictionary.\n";
  std::cout
      << "2. Checksum verification failed (typo in words or wrong order).\n";
  std::cout
      << "3. Incorrect number of words (must be 12, 15, 18, 21, or 24).\n";
  std::cout << "Please try again.\n";
  std::cout << "--------------------------------------------------\n";
}

char render_config_menu(const Config &cfg, std::string_view error_msg ) {

  std::cout << "\033[2J\033[1;1H";

  std::cout << "============================================================\n";
  std::cout << "               [ SEED GENERATION CONFIG ]                   \n";
  std::cout << "============================================================\n";
    if(!error_msg.empty()) {
    std::cout << "\033[1;31m[!] " << error_msg << "\033[0m\n";
    std::cout << "--------------------------------------------\n";
  }
  std::cout << " 1. ENTROPY SOURCE : [ "
            << (!cfg.extra_entropy.empty() ? "OS CSPRNG + USER MIX" : "OS CSPRNG ONLY")
            << " ]\n";
  std::cout << " 2. BIT-LENGTH     : [ " << cfg.bit_length << " bits ("
            << (cfg.bit_length / 32 * 3) << " words) ]\n";
  std::cout << " 3. PASSPHRASE     : [ "
            << (!cfg.passphrase.empty() ? "ENABLED (BIP-39 SALT)" : "DISABLED")
            << " ]\n";
  std::cout << " 4. DERIVATION     : [ " << cfg.derivation_path << " ]\n";

  std::cout << "------------------------------------------------------------\n";

  std::cout << " [\033[1;32mG\033[0m] GENERATE NOW      "
            << "[\033[1;31mB\033[0m] BACK\n";

  std::cout << "============================================================\n";
  std::cout << " > ";
  std::cout.flush();
  std::string input;
input = tech_utils::read_stdin();

  while(input != "1" && input != "2" && input != "3" && input != "4" &&
      input != "b" && input != "e" && input != "g") {
    return render_config_menu(cfg);
  }

  return input.front();
}


void print_wallet_ui(const Wallet &wallet, std::string_view error_msg) {
  std::cout << "\033[2J\033[1;1H";

  std::cout << "\033[1;32m"
            << "===================================================="
            << "\033[0m" << std::endl;
  std::cout << "          \033[1;37mETH CORE WALLET - SESSION ACTIVE\033[0m"
            << std::endl;
  std::cout << "\033[1;32m"
            << "===================================================="
            << "\033[0m" << std::endl;
            if(!error_msg.empty()) {
    std::cout << "\033[1;31m[!] " << error_msg << "\033[0m\n";
    std::cout << "--------------------------------------------\n";
  }
  std::cout << "  [STATUS]   \033[1;32mOnline\033[0m (Syncing...)\n";
    std::cout << "  [BALANCE]  \033[1;36m0.0000 ETH\033[0m (~0.00 USD)\n";
    std::cout << "\033[1;32m----------------------------------------------------\033[0m\n";
    
  std::cout << "  [ADDRESS]  \033[1;33m";
  tech_utils::print_hex(wallet.get_eth_address());
  std::cout << "\033[0m";
  std::cout << "  [PATH]     m/44'/60'/0'/0/" << wallet.getIndex() << std::endl;
  std::cout << "  [NETWORK]  Ethereum Mainnet (Offline Mode)" << std::endl;
  std::cout << "\033[1;32m"
            << "----------------------------------------------------"
            << "\033[0m" << std::endl;

  std::cout << "  1. \033[1;37mSend Transaction\033[0m (Requires RPC)"
            << std::endl;
  std::cout << "  2. \033[1;37mNext Address\033[0m    (Derive Index "
            << wallet.getIndex() + 1 << ")" << std::endl;
  std::cout << "  3. \033[1;37mSwitch Account\033[0m  (BIP-44 Account Level)"
            << std::endl;
  std::cout << "  4. \033[1;37mExport Key\033[0m      (Show Private Hex)"
            << std::endl;
  std::cout << "  5. \033[1;31mLock & Exit\033[0m     (Wipe Memory)"
            << std::endl;

  std::cout << "\033[1;32m"
            << "===================================================="
            << "\033[0m" << std::endl;
  std::cout << ">>> Select action: ";
}

char handle_wallet_ui_input(void) {
  std::string choice = tech_utils::read_stdin();

  while (choice != "1" && choice != "2" && choice != "3" && choice != "4" && choice != "5") {
    std::cout << "Invalid input." << std::endl;
    choice = tech_utils::read_stdin();
  }

  return choice.front();
}


void print_password_setup_ui() {
    std::cout << "\033[2J\033[1;1H";
  std::cout << "\n--------------------------------------------------\n";
  std::cout << "CREATE PASSWORD\n";
  std::cout << "--------------------------------------------------\n";
  std::cout << "This password will encrypt your wallet on disk.\n";
  std::cout << "INPUT IS HIDDEN: No characters will be displayed.\n";
  std::cout << "WARNING: If you forget it, your funds are LOST FOREVER.\n";
  std::cout << "Enter new password:\n";
  std::cout << "> ";
}

void print_confirm_password_setup(void) {
   std::cout << "\033[2J\033[1;1H";
  std::cout << "\n--------------------------------------------------\n";
  std::cout << "CONFIRM MASTER PASSWORD\n";
  std::cout << "--------------------------------------------------\n";
  std::cout << "Please re-enter your password to verify.\n";
  std::cout << "If it doesn't match, you will have to restart.\n";
  std::cout << "Enter password again:\n";
  std::cout << "> ";
}

void print_invalid_password_setup(void) {
  std::cout << "\033[2J\033[1;1H";
  std::cout << "\n--------------------------------------------------\n";
  std::cout << "\033[1;31m[!] ERROR: PASSWORDS DO NOT MATCH\033[0m\n";
  std::cout << "--------------------------------------------------\n";
  std::cout << "The passwords you entered are different.\n";
  std::cout << "Please try again to ensure your funds are safe.\n";
  std::cout << "Enter password again:\n";
  std::cout << "> ";
}

const bytes_data read_and_confirm_password(void) {
print_password_setup_ui();
std::string pass_str, confirm_str;
std::getline(std::cin, pass_str);

print_confirm_password_setup();

std::getline(std::cin, confirm_str);

while(pass_str != confirm_str) {
print_invalid_password_setup();
std::getline(std::cin, confirm_str);
}

bytes_data password_in_bytes(pass_str.begin(), pass_str.end());
OPENSSL_cleanse(pass_str.data(), pass_str.size());
OPENSSL_cleanse(confirm_str.data(), confirm_str.size());

return password_in_bytes;
}
}