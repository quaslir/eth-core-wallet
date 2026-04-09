#include "cli.hpp"
#include "config.hpp"
#include "tech_utils.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <iomanip>
#include <string_view>
namespace cli {
Component createMainMenu(int * selected, std::string_view error_msg) {

  static std::vector<std::string> entries = {
     "  1. Generate New Wallet",
     "  2. Import Wallet",
     "  3. Exit"
  };
  auto menu = Menu(&entries, selected);

  return Renderer(menu, [=, error_msg = std::string(error_msg)] {
    Elements header = {
      text("============================================") | hcenter,
          text("       ETH CORE WALLET v1.0 (Dev Alpha)     ") | bold | hcenter,
          text("============================================") | hcenter,
    };


    Elements info = {
      text("[INFO] System initialized.") | dim,
            separator(),
            text("Please select an operation:"),
    };

    Element error_box = error_msg.empty() ? emptyElement() : vbox({
      separator(),
      text("[!] " + error_msg) | color(Color::Red),
      separator(),
    });

    return vbox({
      vbox(std::move(header)),
      vbox(std::move(info)),
      menu->Render() | border | color(Color::Blue),
      error_box,
      separator(),
      text(">>> Use Arrows to navigate, Enter to select") | dim | hcenter
    }) | border | center;
  });
}


void display_mnemonic(std::string_view mnemonic) {
  auto screen = ScreenInteractive::TerminalOutput();

  auto component = Renderer([&] {
    return render_mnemonic_element(mnemonic);
  });

}

Element render_mnemonic_element(std::string_view mnemonic) {

  return vbox({
    vbox({
      text(" YOUR RECOVERY PHRASE (SEED) ") | bold | center,

    }) | borderDouble | color(Color::Yellow),

    separator(),

    paragraph(std::string{mnemonic}) | hcenter | color(Color::White) | bold, // fix in the future

    separator(),

    vbox({
      text(" WARNING: DO NOT SCREENSHOT! ") | center,
            text(" WRITE IT DOWN ON PAPER NOW! ") | center,
    }) | border | color(Color::Red),

    text(" Press [ENTER] to continue ") | dim | hcenter

  }) | border | center | size(WIDTH, LESS_THAN, 60);
  
}

bool confirm_liability_waiver(void) {
auto screen = ScreenInteractive::TerminalOutput();
std::string error_msg = "";
auto user_input = std::make_shared<std::string>("");
auto confirmation_ui = render_mnemonic_wiping(user_input, error_msg);

auto component = CatchEvent(confirmation_ui, [&](Event event) {
  if(event == Event::Return) {
    if(*user_input == "I AM RESPONSIBLE") {
      screen.Exit();
      return true;
    } 
    return true;
  }

return false;
});

screen.Loop(component);
return true;
}


Component render_mnemonic_wiping(std::shared_ptr<std::string> user_input, std::string_view error_msg) {
  auto input_option = InputOption();
  input_option.multiline = false;

  input_option.transform = [user_input](InputState state) {
    bool is_correct = (*user_input == "I AM RESPONSIBLE");
    state.element |= is_correct ? color(Color::Green) : color(Color::Red);
    return state.element;
  };

  auto field = Input(user_input.get(), "Type here...", input_option);

  return Renderer(field, [=,error_msg = std::string{error_msg}] {
    bool is_correct = (*user_input == "I AM RESPONSIBLE");
    auto input_style = is_correct ? color(Color::Green) : color(Color::Red);


    return vbox({
      vbox({
        text(" [!!!] FINAL LEGAL & SECURITY WARNING [!!!] ") | bold | center
      }) | borderDouble | color(Color::Red),

      separator(),

      text(" 1. This app will NOW WIPE the mnemonic from memory."),
      text(" 2. We have ZERO copies. No database, no logs, no backups."),
      text(" 3. If you didn't write it down, your funds are ALREADY LOST."),

      separator(),

      text(" To proceed, type exactly: ") | hcenter,
      text(" I AM RESPONSIBLE ") | bold | hcenter | inverted,

      separator(),

      hbox({
        text(" >>> "),
        field->Render() | input_style | border,
      }) | hcenter,

      error_msg.empty() ? emptyElement() : text("[!] " + error_msg) | color(Color::Red) | hcenter,
      separator(),

    text(is_correct ? " [ PRESS ENTER TO WIPE MEMORY ] " : " [ WAITING FOR CORRECT INPUT ] ") | bold | hcenter | blink
    }) | border | center | size(WIDTH, LESS_THAN, 70);
  });
}

void request_input_mnemonic_prompt(std::string_view error_msg) {
  std::cout << "\033[2J\033[1;1H";
  std::cout << "\n==================================================\n";
  std::cout << "                [ WALLET IMPORT ]                 \n";
  std::cout << "==================================================\n\n";
  if (!error_msg.empty()) {
    std::cout << "\033[1;31m[!] " << error_msg << "\033[0m\n";
    std::cout << "--------------------------------------------\n";
  }
  std::cout << "STEP 1: ENTER MNEMONIC PHRASE\n";
  std::cout << "Input 12, 15, 18, 21, or 24 words separated by space.\n";
  std::cout
      << "\033[1;30m(Type 'back' or 'exit' to return to main menu)\033[0m\n";
  std::cout << "--------------------------------------------------\n";
  std::cout << "> ";
}

bytes_data request_input_mnemonic(void) {
  std::string error_msg;
  bytes_data mnemonic;

  do {
    OPENSSL_cleanse(mnemonic.data(), mnemonic.size());
    request_input_mnemonic_prompt(error_msg);
    char c;

    while (std::cin.get(c) && c != '\n') {
      mnemonic.push_back(c);
    }

    error_msg = "Mnemonic must not be empty";
  } while (mnemonic.empty());

  std::string_view mnemonic_view(
      reinterpret_cast<const char *>(mnemonic.data()), mnemonic.size());

  if (mnemonic_view == "back" || mnemonic_view == "exit")
    return {};

  return mnemonic;
}

bytes_data request_input_optional_passphrase(void) {
  bytes_data passphrase;
  std::cout << "\033[2J\033[1;1H";
  std::cout << "\n--------------------------------------------------\n";
  std::cout << "STEP 2: ENTER PASSPHRASE (OPTIONAL)\n";
  std::cout << "--------------------------------------------------\n";
  std::cout << "This is an additional security layer\n";
  std::cout << "If you didn't use a passphrase before, leave it empty.\n";
  std::cout << "WARNING: If you forget it, your funds are LOST FOREVER.\n";
  std::cout << "Enter passphrase : \n";
  std::cout << "> ";

  char c;
  while (std::cin.get(c) && c != '\n') {
    passphrase.push_back(c);
  }

  return passphrase;
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

Component render_config_menu(const Config &cfg, int * selected) {

  static std::vector<std::string> entries;

  auto menu = Menu(&entries, selected);


return Renderer(menu, [&cfg, selected, menu] {
  entries = {
     " 1. ENTROPY SOURCE : [ " + std::string{(!cfg.extra_entropy.empty() ? "OS CSPRNG + USER MIX"
                                             : "OS CSPRNG ONLY")},
     " 2. BIT-LENGTH     : [ " + std::string{std::to_string(cfg.bit_length) + " bits ("
              + std::to_string(cfg.bit_length / 32 * 3) + " words) ]"},
     " 3. PASSPHRASE     : [ " + std::string{(!cfg.passphrase.empty() ? "ENABLED (BIP-39 SALT)"
                                          : "DISABLED")},
    " 4. DERIVATION     : [ " + cfg.derivation_path      , 
    " [B] Back to Main Menu",
            " [G] Generate Wallet"                                
  };

  return vbox({
    text(" CONFIGURATION SETTINGS ") | bold | hcenter | color(Color::Cyan),
    separator(),
    vbox({
      [&]() {
        Elements items;

        for(size_t i = 0; i < entries.size(); i++) {
          auto item = text(entries[i]);

          if(static_cast<int>(i) == *selected) {
            item = item | inverted | color(Color::Yellow);
          }

          items.push_back(item);
        }

        return vbox(std::move(items));
      }()
    }) | border,

    separator(),
    text(" Use Arrows to navigate, Enter to toggle/select") | dim | hcenter
    
  }) | center;
});
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
  if (!error_msg.empty()) {
    std::cout << "\033[1;31m[!] " << error_msg << "\033[0m\n";
    std::cout << "--------------------------------------------\n";
  }
  std::cout << "  [STATUS]   \033[1;32mOnline\033[0m (Syncing...)\n";
  std::cout << "  [BALANCE]  \033[1;36m" << (wallet.get_balance().empty() ? "0" : wallet.get_balance())  << " ETH" <<"\033[0m (~0.00 USD)\n";
  std::cout << "\033[1;32m----------------------------------------------------"
               "\033[0m\n";

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
  std::cout << "  3. \033[1;37mPrevious address\033[0m  (Derive Index "
            << wallet.getIndex() - 1 << ")" << std::endl;
  std::cout << "  4. \033[1;37mExport Key\033[0m      (Show Private Hex)"
            << std::endl;
  std::cout << "  5. \033[1;31mLock & Exit\033[0m     (Wipe Memory)"
            << std::endl;

  std::cout << "\033[1;32m"
            << "===================================================="
            << "\033[0m" << std::endl;
  std::cout << ">>> Select action: ";
}

int handle_wallet_ui_input(const Wallet &wallet) {
  std::string error_msg, choice;
  do {
    print_wallet_ui(wallet, error_msg);
    choice = tech_utils::read_stdin();

    if (choice.empty() || (choice != "1" && choice != "2" && choice != "3" &&
                           choice != "4" && choice != "5")) {
      error_msg = "Invalid choice.";
      continue;
    }

    return choice.front() - '0';
  } while (1);

  return -1;
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
  pass_str = tech_utils::read_stdin();

  print_confirm_password_setup();

  confirm_str = tech_utils::read_stdin();

  while (pass_str != confirm_str) {
    print_invalid_password_setup();
    confirm_str = tech_utils::read_stdin();
  }

  bytes_data password_in_bytes(pass_str.begin(), pass_str.end());
  OPENSSL_cleanse(pass_str.data(), pass_str.size());
  OPENSSL_cleanse(confirm_str.data(), confirm_str.size());

  return password_in_bytes;
}

const bytes_data request_unlock_password(size_t attempts, size_t max_attempts) {
  std::cout << "\033[2J\033[1;1H"; // Clear screen

  std::cout << "\n  \033[1mUnlock Wallet\033[0m\n";
  std::cout << "  Please enter your password to decrypt the local storage.\n";
  std::cout
      << "  Your keys remain encrypted until a valid password is provided.\n\n";

  if (attempts > 0) {
    int remaining = max_attempts - attempts;

    std::cout << "  \033[1;31m[!] INVALID PASSWORD\033[0m\n";

    if (remaining > 1) {
      std::cout << "  \033[1;33mCareful: " << remaining
                << " attempts remaining.\033[0m\n\n";
    } else {

      std::cout << "  \033[1;5;31m[WARNING] FINAL ATTEMPT. NEXT FAILURE WILL "
                   "WIPE DATA.\033[0m\n\n";
    }
  }

  std::cout << "  Password: ";

  std::string buffer = tech_utils::read_stdin();
  bytes_data password(buffer.begin(), buffer.end());

  OPENSSL_cleanse(buffer.data(), buffer.size());

  return password;
}

void show_self_destruct(void) {
  std::cout << "\033[2J\033[1;1H"; // Clear
  std::cout << "\n  \033[1;31m[CRITICAL] ACCESS DENIED: 3/3 ATTEMPTS\033[0m\n";
  std::cout << "  ------------------------------------------\n";
  std::cout << "  Action: \033[1mDELETING KEYSTORE\033[0m\n";

  std::cout << "  File shredded. Use your seed phrase to recover.\n";
  std::cout << "  System exit.\n\n";
}

bool confirm_danger_action(void) {
  std::cout << "\033[2J\033[1;1H";
  std::cout << "\n  \033[1;41m  !!! SECURITY WARNING !!!  \033[0m\n";
  std::cout << "  ------------------------------------------\n";

  std::cout << "  1. Anyone with this key has \033[1;31mFULL ACCESS\033[0m to "
               "your funds.\n";
  std::cout << "  2. Never share this with anyone, including support.\n";
  std::cout << "  3. Ensure no one is looking at your screen right now.\n";
  std::cout << "  4. Do not take screenshots or copy to clipboard.\n";
  std::cout << "  ------------------------------------------\n";
  std::cout << "  Are you absolutely sure? (type \033[1;32m'CONFIRM'\033[0m to "
               "proceed): ";

  std::string input = tech_utils::read_stdin();

  return (input == "CONFIRM");
}

void display_private_key(const bytes_data &priv_key) {
  std::cout << "\n  \033[1;33m[ YOUR PRIVATE KEY (HEX) ]\033[0m\n";
  std::cout
      << "  ------------------------------------------------------------\n";

  tech_utils::print_hex(priv_key);

  std::cout
      << "  ------------------------------------------------------------\n";
  std::cout << "  \033[3mWrite it down on paper and keep it offline.\033[0m\n";
  std::cout << "  \033[1;31mPress ENTER immediately to hide this key and "
               "continue...\033[0m";
  std::string buffer;
  if (std::getline(std::cin, buffer)) {
    std::cout << "\033[2J\033[1;1H";
  }
}
} // namespace cli