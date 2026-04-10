#include "cli.hpp"
#include "config.hpp"
#include "tech_utils.hpp"

#include <cstddef>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <ftxui/screen/color.hpp>
#include <memory>
#include <openssl/err.h>
#include <string>
#include <string_view>


void CLI::load(void) {
  auto main_menu = create_main_menu();
  auto mnemonic_display = render_mnemonic_element();
  auto mnenonic_wiping_confirmation = render_mnemonic_wiping();
  auto config_menu = render_config_menu();
  auto password_unlock = render_request_unlock_password();
  auto wallet_ui = print_wallet_ui();

  auto root_container =
      Container::Tab({main_menu, config_menu, mnenonic_wiping_confirmation,
                      mnemonic_display, password_unlock, wallet_ui},
                     &this->active_tab);

  screen.Loop(root_container);
}

Component CLI::create_main_menu(void) {

  static std::vector<std::string> entries = {"  1. Generate New Wallet",
                                             "  2. Import Wallet", "  3. Exit"};
  static int selected = 0;
  auto menu = Menu(&entries, &selected);

  auto component = ftxui::CatchEvent(menu, [&](ftxui::Event event) {

    if (event == ftxui::Event::Character('q') ||
        event == ftxui::Event::Character('Q')) {
      selected = 2;
      on_main_menu(selected);
      return true;
    }

    if (event == ftxui::Event::Return) {
      on_main_menu(selected + 1);
      return true;
    }
    return false;
  });

  return Renderer(component, [=] {
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

    return vbox({vbox(std::move(header)), vbox(std::move(info)),
                 menu->Render() | border | color(Color::Blue), separator(),
                 text(">>> Use Arrows to navigate, Enter to select") | dim |
                     hcenter}) |
           border | center;
  });
}

Component CLI::render_mnemonic_element(void) {

 auto button = Button(" [ PRESS ENTER TO CONTINUE ] ", [&] {
  this->set_active_tab(2);
 }, ButtonOption::Ascii());


  return Renderer(button, [&, button] {
    std::string mnemonic;
    if(get_mnemonic) {
      mnemonic = get_mnemonic();
    } else
      mnemonic = "";
    return vbox({vbox({
                     text(" YOUR RECOVERY PHRASE (SEED) ") | bold | center,

                 }) | borderDouble |
                     color(Color::Yellow),

                 separator(),

                 paragraph(std::string{mnemonic}) | hcenter |
                     color(Color::White) | bold, // fix in the future

                 separator(),

                 vbox({
                     text(" WARNING: DO NOT SCREENSHOT! ") | center,
                     text(" WRITE IT DOWN ON PAPER NOW! ") | center,
                 }) | border |
                     color(Color::Red),

            separator(),
            button->Render() | hcenter | bold | focus

           }) |
           border | center | size(WIDTH, LESS_THAN, 60);
  });


}

Component CLI::render_mnemonic_wiping(void) {

  auto user_input = std::make_shared<std::string>("");

  auto input_option = InputOption();
  input_option.multiline = false;

  input_option.transform = [user_input](InputState state) {
    bool is_correct = (*user_input == "I AM RESPONSIBLE");
    state.element |= is_correct ? color(Color::Green) : color(Color::Red);
    return state.element;
  };

  auto field = Input(user_input.get(), "Type here...", input_option);

  auto confirmation_ui = Renderer(field, [=] {
    bool is_correct = (*user_input == "I AM RESPONSIBLE");
    auto input_style = is_correct ? color(Color::Green) : color(Color::Red);

    return vbox({vbox({text(" [!!!] FINAL LEGAL & SECURITY WARNING [!!!] ") |
                       bold | center}) |
                     borderDouble | color(Color::Red),

                 separator(),

                 text(" 1. This app will NOW WIPE the mnemonic from memory."),
                 text(" 2. We have ZERO copies. No database, no logs, no "
                      "backups."),
                 text(" 3. If you didn't write it down, your funds are ALREADY "
                      "LOST."),

                 separator(),

                 text(" To proceed, type exactly: ") | hcenter,
                 text(" I AM RESPONSIBLE ") | bold | hcenter | inverted,

                 separator(),

                 hbox({
                     text(" >>> "),
                     field->Render() | input_style | border,
                 }) | hcenter,

                 separator(),

                 text(is_correct ? " [ PRESS ENTER TO WIPE MEMORY ] "
                                 : " [ WAITING FOR CORRECT INPUT ] ") |
                     bold | hcenter | blink}) |
           border | center | size(WIDTH, LESS_THAN, 70);
  });

  return CatchEvent(confirmation_ui, [&](Event event) {
    if (event == Event::Return) {
      if (*user_input == "I AM RESPONSIBLE") {
        return true;
      }
      return true;
    }

    return false;
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

bytes_data CLI::request_input_mnemonic(void) {
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

bytes_data CLI::request_input_optional_passphrase(void) {
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

void CLI::incorrect_mnemonic_text(void) {
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

Component CLI::render_config_menu(void) {

  const Config &cfg = get_config();
  static std::vector<std::string> entries;
  static int selected = 0;
  auto menu = Menu(&entries, &selected);

  auto component = Renderer(menu, [&cfg, menu] {
    entries = {
        " 1. ENTROPY SOURCE : [ " +
            std::string{(!cfg.extra_entropy.empty() ? "OS CSPRNG + USER MIX"
                                                    : "OS CSPRNG ONLY")},
        " 2. BIT-LENGTH     : [ " +
            std::string{std::to_string(cfg.bit_length) + " bits (" +
                        std::to_string(cfg.bit_length / 32 * 3) + " words) ]"},
        " 3. PASSPHRASE     : [ " +
            std::string{(!cfg.passphrase.empty() ? "ENABLED (BIP-39 SALT)"
                                                 : "DISABLED")},
        " 4. DERIVATION     : [ " + cfg.derivation_path,
        " [B] Back to Main Menu",
        " [G] Generate Wallet"};

    return vbox({text(" CONFIGURATION SETTINGS ") | bold | hcenter |
                     color(Color::Cyan),
                 separator(),
                 vbox({[&]() {
                   Elements items;

                   for (size_t i = 0; i < entries.size(); i++) {
                     auto item = text(entries[i]);

                     if (static_cast<int>(i) == selected) {
                       item = item | inverted | color(Color::Yellow);
                     }

                     items.push_back(item);
                   }

                   return vbox(std::move(items));
                 }()}) |
                     border,

                 separator(),
                 text(" Use Arrows to navigate, Enter to toggle/select") | dim |
                     hcenter

           }) |
           center;
  });
  return CatchEvent(component, [&](ftxui::Event event) {
    if (event == ftxui::Event::Return) {
      int choice = selected + 1;

      handle_config_menu(choice);

      return true;
    }
    return false;
  });
}

Component CLI::print_wallet_ui(void) {
  int selected;
  static std::vector<std::string> entries;
  auto menu = Menu(&entries, &selected);
  entries = {" 1. Send Transaction ", " 2. Next Address      ",
             " 3. Previous Address  ", " 4. Export Key        ",
             " 5. Lock & Exit       "};

  auto walletUI = Renderer(menu, [&] {
    const Wallet &wallet = get_wallet();
    std::string balance =
        wallet.get_balance().empty() ? "0.00" : wallet.get_balance();
    std::string address = tech_utils::to_hex(wallet.get_eth_address());
    return vbox({

               vbox({
                   text("ETH CORE WALLET ") | bold | hcenter,
                   text("SESSION ACTIVE ") | dim | hcenter,
               }) | borderDouble |
                   color(Color::Cyan),

               vbox({
                   hbox({text(" STATUS: "),
                         text("Online (Syncing...)") | color(Color::Green)}),
                   hbox({text(" BALANCE: "),
                         text(balance + " ETH") | color(Color::Yellow)}),
                   hbox({text(" ADDRESS: "),
                         text("0x" + address) | color(Color::GrayDark)}),
                   hbox({text(" NETWORK: "),
                         text("Ethereum Mainnet") | color(Color::Green)}),
               }),

               separator(),
               vbox({
                   text(" SELECT OPERATION: ") | bold,
                   menu->Render() | color(Color::BlueLight),
               }) | border,

               text(">>> Use Arrows to navigate, Enter to select") | dim |
                   hcenter,
           }) |
           border | center | size(WIDTH, LESS_THAN, 62);
  });

  return CatchEvent(walletUI, [&](Event event) {
    if (event == Event::Return) {
      int choice = selected + 1;
      if (choice == 1 || choice == 2 || choice == 3 || choice == 4 ||
          choice == 5) {
        return true;
      }
      return true;
    }

    return false;
  });
}

Component CLI::render_password_setup(std::shared_ptr<std::string> user_input) {

  auto input_option = InputOption();
  input_option.multiline = false;

  auto field = Input(user_input.get(), "Enter password...", input_option);

  return Renderer(field, [=] {
    return vbox(
        {text("CREATE PASSWORD") | bold | center, separator(),

         text("This password will encrypt your wallet on disk."),
         text("INPUT IS HIDDEN: No characters will be displayed."),
         text("WARNING: If you forget it, your funds are LOST FOREVER.") |
             color(Color::Red),

         separator(),
         hbox({
             text(" >>> "),
             field->Render() | border,
         }) | hcenter});
  });
}

Component
CLI::render_confirm_password_setup(std::shared_ptr<std::string> user_input,
                                   bool incorrect) {
  auto input_option = InputOption();
  input_option.multiline = false;

  auto field = Input(user_input.get(), "Enter password...", input_option);

  return Renderer(field, [=] {
    std::vector<std::string> password_text = !incorrect ? std::vector<std::string>{
    "CONFIRM MASTER PASSWORD",
    "Please re-enter your password to verify.",
    "If it doesn't match, you will have to restart.",
  } : std::vector<std::string> {
    "[!] ERROR: PASSWORDS DO NOT MATCH",
    "Please try again to ensure your funds are safe."
  };

    Color text_color = !incorrect ? Color::Cyan : Color::Red;

    Elements elements;

    for (const auto &line : password_text) {
      elements.push_back(text(line) | hcenter);
    }

    return vbox({vbox(std::move(elements)) | color(text_color) | bold,
                 separator(), hbox({text(" >>> "), field->Render() | border}),
                 separator(),
                 text("Press [ENTER] to confirm") | dim | hcenter}) |
           border | center;
  });
}

const bytes_data CLI::read_and_confirm_password(void) {

  auto pass_str = std::make_shared<std::string>("");
  auto first_input_ui = render_password_setup(pass_str);

  auto first_stage = CatchEvent(first_input_ui, [&](Event event) {
    if (event == Event::Return && !pass_str->empty()) {
      return true;
    }

    return false;
  });

  screen.Loop(first_stage);

  auto confirm_str = std::make_shared<std::string>("");
  bool is_incorrect = false;

  for (;;) {
    auto confirm_ui = render_confirm_password_setup(confirm_str, is_incorrect);

    auto second_stage = CatchEvent(confirm_ui, [&](Event event) {
      if (event == Event::Return) {
        if (*pass_str == *confirm_str) {
        } else {
          is_incorrect = true;
          confirm_str->clear();
        }

        return true;
      }

      return false;
    });
    screen.Loop(second_stage);
    if (*pass_str == *confirm_str)
      break;
  }

  bytes_data password_in_bytes(pass_str->begin(), pass_str->end());
  // OPENSSL_cleanse(pass_str.get(), pass_str.size());
  // OPENSSL_cleanse(confirm_str.data(), confirm_str.size());

  return password_in_bytes;
}

Component CLI::render_request_unlock_password(void) {
  auto input_option = InputOption();
  size_t attempts = 0;//get_attempts();
  size_t max_attempts = 3;//get_max_attempts();
  std::shared_ptr<std::string> user_input = std::make_shared<std::string>("");
  input_option.multiline = false;
  input_option.password = true;
  auto field = Input(user_input.get(), "Enter password...", input_option);

  auto component = CatchEvent(field, [&](Event event) {
    if (event == Event::Return && !user_input->empty()) {
      return true;
    }

    return false;
  });

  return Renderer(component, [=] {
    int remaining = max_attempts - attempts;
    Elements status_info;
    if (attempts > 0) {
      status_info.push_back(text("[!] INVALID PASSWORD") | bold |
                            color(Color::Red) | hcenter);
    }

    if (remaining == 1) {
      status_info.push_back(text("[WARNING] FINAL ATTEMPT. NEXT FAILURE WILL "
                                 "WIPE DATA.") |
                            bold | color(Color::RedLight) | blink | hcenter);
    } else if (remaining > 0 && attempts > 0) {
      status_info.push_back(text("Careful: " + std::to_string(remaining) +
                                 " attempts remaining.") |
                            dim | hcenter);
    }
    return vbox({vbox({text(" Unlock Wallet ") | bold | center}) |
                     borderDouble | color(Color::Cyan),

                 separator(),

                 vbox({
                     text(" Please enter your password to decrypt the local "
                          "storage. ") |
                         hcenter,
                     text(" Your keys remain encrypted until a valid password "
                          "is provided. ") |
                         hcenter | dim,
                 }),

                 vbox(std::move(status_info)),

                 hbox({
                     text(" >>> "),
                     field->Render() | border | center |
                         size(WIDTH, LESS_THAN, 70),
                 }) | hcenter,

                 separator(),

                 text(" Press [ENTER] to unlock") | hcenter | dim

           }) |
           border | center | size(WIDTH, LESS_THAN, 70);
  });
}

void show_self_destruct(void) {
  std::cout << "\033[2J\033[1;1H"; // Clear
  std::cout << "\n  \033[1;31m[CRITICAL] ACCESS DENIED: 3/3 ATTEMPTS\033[0m\n";
  std::cout << "  ------------------------------------------\n";
  std::cout << "  Action: \033[1mDELETING KEYSTORE\033[0m\n";

  std::cout << "  File shredded. Use your seed phrase to recover.\n";
  std::cout << "  System exit.\n\n";
}

bool CLI::confirm_danger_action(void) {
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

void CLI::display_private_key(const bytes_data &priv_key) {
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

void CLI::set_active_tab(int tab) {
active_tab = tab;
screen.Post(Event::Custom);
}