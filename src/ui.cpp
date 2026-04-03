#include "ui.hpp"
#include "cli.hpp"
#include <iostream>
#include "config.hpp"
void UserInterface::load(void) {
    cli::print_welcome_message();
    make_choice_from_welcome_message();
    cli::print_wallet_ui(wallet);
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


void UserInterface::handle_wallet_creation(void) {
    cli::render_config_menu(config);
   bytes_data mnemonic = wallet.prepare_mnemonic(128);
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

bool UserInterface::handle_seed_generation_config(void) {
    char choice = cli::render_config_menu(config);
    if(choice == 'g') return 1;
    else if(choice == 'b') return 0;

    else if(choice == '1') {
        
    }
    else if(choice == '2') {

    }
    else if(choice == '3') {

    }
    else if(choice == '4') {

    }

}