#include "config.hpp"
#include "crypto_utils.hpp"
#include "tech_utils.hpp"
#include <iostream>

#define ERROR_MSG "\033[1;31m[!] Invalid choice.\033[0m\n\n"

void Config::handle_bit_length(void) {
  std::string choice, error_msg;
  do {
    render_bit_length_menu(error_msg);
    choice = tech_utils::read_stdin();
    error_msg = ERROR_MSG;
  } while (choice.empty() || (choice != "b" && choice != "1" && choice != "2"));
  if (choice == "b")
    return;
  if (choice == "1")
    bit_length = 128;
  else if (choice == "2")
    bit_length = 256;
}

void Config::render_passphrase_menu(std::string_view error_msg) const {
  std::cout << "\033[2J\033[1;1H";
  std::cout << "============================================================\n"
            << "             [ BIP-39 PASSPHRASE SETUP ]                    \n"
            << "============================================================\n"
            << (!error_msg.empty() ? error_msg : "")
            << " [1] DISABLED (Standard Mode)\n"
            << "     Wallet depends ONLY on your 12/24 words.\n\n"
            << " [2] ENABLED (Advanced / 25th Word)\n"
            << "     Adds a custom salt to your mnemonic. Creates a \n"
            << "     HIDDEN wallet. Requires absolute precision.\n\n"
            << " [!] WARNING: NO RECOVERY POSSIBLE WITHOUT PASSPHRASE.\n"
            << "     Lost passphrase = Lost funds. Forever.\n"
            << "------------------------------------------------------------\n"
            << " [B] BACK TO CONFIG\n"
            << "============================================================\n"
            << " > ";
}

void Config::render_passphrase_input_screen() const {
  std::cout << "\033[2J\033[1;1H";
  std::cout << "============================================================\n"
            << "             [ ENTER SECRET PASSPHRASE ]                    \n"
            << "============================================================\n"
            << " [!] CAUTION: The passphrase is CASE-SENSITIVE.\n"
            << "     'Apple' and 'apple' will lead to DIFFERENT wallets.\n\n"
            << " > Input: ";
  std::cout.flush();
}

void Config::handle_use_passphrase(void) {
  std::string choice, error_msg;
  do {
    render_passphrase_menu(error_msg);
    choice = tech_utils::read_stdin();
    error_msg = ERROR_MSG;
  } while (choice.empty() || (choice != "b" && choice != "1" && choice != "2"));

  if (choice == "b")
    return;
  else if (choice == "1") {
    OPENSSL_cleanse(passphrase.data(), passphrase.size());
    passphrase.clear();
  }

  else if (choice == "2") {
    render_passphrase_input_screen();
    std::string input = tech_utils::read_stdin();
    passphrase.assign(input.begin(), input.end());
    OPENSSL_cleanse(input.data(), input.size());
  }
}

void Config::render_bit_length_menu(std::string_view error_msg) const {
  std::cout << "\033[2J\033[1;1H";
  std::cout << "============================================================\n"
            << "             [        SELECT ENTROPY LENGTH ]              \n"
            << "============================================================\n"
            << (!error_msg.empty() ? error_msg : "")
            << " [1] 128 BITS (12 WORDS)\n"
            << "     Standard security. Ideal for \"Hot\" wallets and\n"
            << "     frequent transactions. Easy to write and store.\n\n"
            << " [2] 256 BITS (24 WORDS)\n"
            << "     Military-grade security. Recommended for \"Cold\"\n"
            << "     storage. Impossible to brute-force, even in theory.\n"
            << "------------------------------------------------------------\n"
            << " [B] BACK TO CONFIG\n"
            << "============================================================\n"
            << " > ";
}

void Config::render_extra_entropy_menu(std::string_view error_msg) const {
  std::cout << "\033[2J\033[1;1H";
  std::cout << "============================================================\n"
            << (!error_msg.empty() ? error_msg : "")
            << " [1] SYSTEM ONLY (OS CSPRNG)\n"
            << "     Standard mode. Uses /dev/urandom or BCryptGenRandom.\n"
            << "     Reliable and tested by millions of users.\n\n"
            << " [2] SYSTEM + USER MIX (SHA-256 Hybrid)\n"
            << "     Paranoid mode. Mixes OS random with your secret \n"
            << "     input using SHA-256. Protects against OS backdoors.\n\n"
            << "------------------------------------------------------------\n"
            << " [B] BACK TO CONFIG\n"
            << "============================================================\n"
            << " > ";
}

void Config::render_input_extra_entropy_menu(void) const {
  std::cout << "\033[2J\033[1;1H";
  std::cout << "============================================================\n"
            << "             [ INJECT CUSTOM ENTROPY ]                      \n"
            << "============================================================\n"
            << " Enter any random string, dice rolls, or secret words.\n"
            << " This input will be hashed (SHA-256) with system entropy.\n\n"
            << " [!] TIP: You can literally smash your keyboard or type\n"
            << "     a sentence only you know. This is NOT a password,\n"
            << "     it's a one-time 'chaos' injection.\n"
            << "------------------------------------------------------------\n"
            << " > Input: ";
}

void Config::handle_extra_entropy(void) {
  std::string choice, error_msg;
  do {
    render_extra_entropy_menu(error_msg);
    choice = tech_utils::read_stdin();
    error_msg = ERROR_MSG;
  } while (
      (choice.empty() || (choice != "b" && choice != "1" && choice != "2")));

  if (choice == "b")
    return;
  else if (choice == "1") {
    OPENSSL_cleanse(extra_entropy.data(), extra_entropy.size());
    extra_entropy.clear();
    return;
  }

  else if (choice == "2") {
    render_input_extra_entropy_menu();

    std::string input = tech_utils::read_stdin();
    extra_entropy.assign(input.begin(), input.end());
    OPENSSL_cleanse(input.data(), input.size());
  }
}

void Config::render_derivation_menu(std::string_view error_msg) const {
  std::cout << "\033[2J\033[1;1H";
  std::cout << "============================================================\n"
            << (!error_msg.empty() ? error_msg : "")
            << " [1] ETHEREUM STANDARD (BIP-44)\n"
            << "     m/44'/60'/0'/0/0\n"
            << "     Recommended for MetaMask, MyEtherWallet, and Ledger.\n\n"
            << " [2] CUSTOM PATH (Advanced)\n"
            << "     Manually specify account, change, or address index.\n"
            << "     Use only if you know what you are doing!\n\n"
            << " [!] NOTE: Different paths result in DIFFERENT addresses\n"
            << "     even with the same 12/24 words.\n"
            << "------------------------------------------------------------\n"
            << " [B] BACK TO CONFIG\n"
            << "============================================================\n"
            << " > ";
}

void Config::render_custom_path_input(std::string_view error_msg) const {
  std::cout << "\033[2J\033[1;1H";
  std::cout
      << "============================================================\n"
      << "             [ ENTER CUSTOM DERIVATION PATH ]               \n"
      << "============================================================\n"
      << (!error_msg.empty() ? error_msg : "")
      << " Format follows BIP-44 standard:\n"
      << " m / purpose' / coin_type' / account' / change / address_index\n"
      << " \n"
      << " Example (Ethereum): m/44'/60'/0'/0/0\n"
      << " Example (Second account): m/44'/60'/1'/0/0\n"
      << "------------------------------------------------------------\n"
      << " [!] WARNING: Small changes in path lead to completely \n"
      << "     different addresses. Write it down if it's non-standard.\n"
      << "------------------------------------------------------------\n"
      << " > Path: ";
}

void Config::handle_derivation_path(void) {
  std::string choice, error_msg;
  do {
    render_derivation_menu(error_msg);
    choice = tech_utils::read_stdin();
    error_msg = ERROR_MSG;
  } while (choice.empty() || (choice != "1" && choice != "2" && choice != "b"));

  if (choice == "b")
    return;
  if (choice == "1") {
    derivation_path = "m/44'/60'/0'/0/0";
  }

  if (choice == "2") {
    error_msg.clear();
    render_custom_path_input(error_msg);
    std::string path = tech_utils::read_stdin();

    while (!crypto_utils::is_valid_derive_path(path)) {
      error_msg = "Invalid path.";
      render_custom_path_input(error_msg);
      path = tech_utils::read_stdin();
    }

    derivation_path = path;
    path.clear();
  }
}