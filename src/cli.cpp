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
#include <iostream>

#include <memory>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <string>
#include <string_view>

void CLI::set_actions(IWalletActions * act) {
    actions = act;
}

void CLI::load(void) {
  auto main_menu = create_main_menu();
  auto mnemonic_display = render_mnemonic_element();
  auto mnenonic_wiping_confirmation = render_mnemonic_wiping();
  auto import_wallet_ui = render_import_mnemonic_component();
  auto optional_passphrase = render_input_optional_passphrase_component();
  auto config_menu = render_config_menu();
  auto set_password_component = render_password_setup();
  auto confirm_password_component = render_confirm_password_setup();
  auto password_unlock = render_request_unlock_password();
  auto wallet_ui = print_wallet_ui();
  auto bit_length_selection_config = set_bit_length();
  auto root_container = Container::Tab(
      {main_menu, config_menu, import_wallet_ui, optional_passphrase,
       mnemonic_display, mnenonic_wiping_confirmation, set_password_component,
       confirm_password_component, wallet_ui, password_unlock, bit_length_selection_config},
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
      actions->on_main_menu(selected);
      return true;
    }

    if (event == ftxui::Event::Return) {
      actions->on_main_menu(selected + 1);
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

  auto button = Button(
      " [ PRESS ENTER TO CONTINUE ] ",
      [&] { this->set_active_tab(MNEMONIC_WIPING); }, ButtonOption::Ascii());

  return Renderer(button, [&, button] {
    std::string mnemonic = actions->get_mnemonic();

    auto box = vbox({vbox({
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

                     separator(), button->Render() | hcenter | bold | focus

               }) |
               border | center | size(WIDTH, LESS_THAN, 60);

    return to_center(box);
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

  input_option.on_enter = [=, this] {
    if (*user_input == "I AM RESPONSIBLE") {
      set_active_tab(SET_PASSWORD);
    }
  };

  auto field = Input(user_input.get(), "Type here...", input_option);
  field->TakeFocus();

  return Renderer(field, [=, this] {
    bool is_correct = (*user_input == "I AM RESPONSIBLE");
    auto input_style = is_correct ? color(Color::Green) : color(Color::Red);

    auto box =
        vbox({vbox({text(" [!!!] FINAL LEGAL & SECURITY WARNING [!!!] ") |
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

    return to_center(box);
  });
}

Component CLI::render_import_mnemonic_component(void) {
  auto is_incorrect = std::make_shared<bool>(0);
  auto user_input = std::make_shared<std::string>("");

  auto input_option = InputOption();
  input_option.multiline = false;

  auto field = Input(user_input.get(), "Enter mnemonic....", input_option);
  field->TakeFocus();

  auto component = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return) {
      if (!user_input->empty()) {
        if (actions->check_mnemonic(std::string_view(*user_input))) {
          actions->set_mnemonic(*user_input); // fix
          OPENSSL_cleanse(user_input->data(), user_input->size());
          set_active_tab(ENTER_OPTIONAL_PASSPHRASE_MENU);
        } else
          *is_incorrect = true;
        return true;
      } else
        *is_incorrect = true;

      return true;
    }

    return false;
  });

  return Renderer(component, [=, this] {
    Element error_box = emptyElement();
    if (*is_incorrect) {
      error_box = vbox({

          vbox({
              text(" ⚠  ERROR ") | bgcolor(Color::Red) | color(Color::White) |
                  bold,
              text("[!] Invalid Mnemonic Phrase") | color(Color::Red) | bold,
          }),
          separatorLight() | color(Color::Red),
          vbox({

              text("Your phrase could not be verified. Check for:"),
              text("1. One or more words are not from the BIP-39 dictionary."),
              text("2. Checksum verification failed (typo in words or wrong "
                   "order)."),
              text("3. Incorrect number of words (must be 12, 15, 18, 21, or "
                   "24)."),
              separatorDouble() | color(Color::Red),
          }) | border |
              color(Color::Red),
      });
    }

    auto box =
        vbox({
            vbox({
                text(" [ WALLET IMPORT ] ") | bold | center,
            }) | borderDouble,

            error_box,

            vbox({
                text(" STEP 1: ENTER MNEMONIC PHRASE ") | color(Color::Cyan),
                text("Input 12, 15, 18, 21, or 24 words separated by "
                     "space.") |
                    dim,
                text("(Type 'back' or 'exit' to return to main menu)") |
                    color(Color::DarkSlateGray1),
            }),

            separator(),

            hbox({
                text(" > ") | bold | color(Color::Yellow),
                field->Render() | flex,
            }),

            separator(),
            text(" [ WAITING FOR INPUT ] ") | dim | center,
        }) |
        border | center | size(WIDTH, LESS_THAN, 70);

    return to_center(box);
  });
}

Component CLI::render_input_optional_passphrase_component(void) {
  auto user_input = std::make_shared<std::string>("");

  auto input_option = InputOption();
  input_option.multiline = false;
  input_option.password = true;

  auto field = Input(user_input.get(), "Enter passphrase...", input_option);
  field->TakeFocus();

  auto component = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return) {
      actions->set_passphrase(*user_input);
      OPENSSL_cleanse(user_input->data(), user_input->size());
      actions->import_wallet();
      set_active_tab(SET_PASSWORD);
      return true;
    }

    return false;
  });

  return Renderer(component, [=, this] {
    auto box = vbox({
        text("STEP 2: ENTER PASSPHRASE (OPTIONAL)") | color(Color::Cyan) | bold,
        separator(),
        text("This is an additional security layer"),
        text("If you didn't use a passphrase before, leave it empty."),
        text("WARNING: If you forget it, your funds are LOST FOREVER.") |
            color(Color::Red) | bold,
        separator(),

        component->Render() | border,
    });

    return to_center(box);
  });
}

Component CLI::render_config_menu(void) {

  const Config &cfg = actions->get_config();
  static std::vector<std::string> entries;
  static int selected = 0;
  auto menu = Menu(&entries, &selected);

  auto component = Renderer(menu, [&cfg, menu, this] {
    entries = {
        " 1. ENTROPY SOURCE : [ " +
            std::string{(!cfg.extra_entropy.empty() ? "OS CSPRNG + USER MIX"
                                                    : "OS CSPRNG ONLY")} +
            " ]",
        " 2. BIT-LENGTH     : [ " +
            std::string{std::to_string(cfg.bit_length) + " bits (" +
                        std::to_string(cfg.bit_length / 32 * 3) + " words) ]"},
        " 3. PASSPHRASE     : [ " +
            std::string{(!cfg.passphrase.empty() ? "ENABLED (BIP-39 SALT)"
                                                 : "DISABLED")} +
            " ]",
        " 4. DERIVATION     : [ " + cfg.derivation_path + " ]",
        " [B] Back to Main Menu",
        " [G] Generate Wallet"};

    auto box = vbox({text(" CONFIGURATION SETTINGS ") | bold | hcenter |
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
                     text(" Use Arrows to navigate, Enter to toggle/select") |
                         dim | hcenter

               }) |
               center;

    return to_center(box);
  });
  return CatchEvent(component, [&](ftxui::Event event) {
    if (event == ftxui::Event::Return) {
      int choice = selected + 1;

      actions->handle_config_menu(choice);

      return true;
    }
    return false;
  });
}

Component CLI::print_wallet_ui(void) {
  static int selected = 0;
  static std::vector<std::string> entries;
  auto menu = Menu(&entries, &selected);
  entries = {" 1. Send Transaction ", " 2. Next Address      ",
             " 3. Previous Address  ", " 4. Export Key        ",
             " 5. Lock & Exit       "};

  auto walletUI = Renderer(menu, [=, this] {
    const Wallet &wallet = actions->get_wallet(); //!!!
    std::string balance =
        wallet.get_balance().empty() ? "0.00" : wallet.get_balance();
    std::string address =
        tech_utils::to_hex(wallet.get_eth_address()); // NEEDS TO BE CACHED

    auto info_line = [](const std::string &label, const std::string &value,
                        Color color_) {
      return hbox(
          {text(label) | size(WIDTH, EQUAL, 12), text(value) | color(color_)});
    };
    auto box =
        vbox({

            vbox({
                text("ETH CORE WALLET ") | bold | hcenter,
                text("SESSION ACTIVE ") | dim | hcenter,
            }) | borderDouble |
                color(Color::Cyan),

            vbox({
                info_line(" STATUS: ", "Online (Syncing...)", Color::Green),
                info_line(" BALANCE: ", balance + " ETH", Color::Yellow),
                info_line(" ADDRESS: ", "0x" + address, Color::DarkSlateGray1),
                info_line(" NETWORK: ", "Ethereum Mainnet", Color::Green),

            }),

            separator(),
            vbox({
                text(" SELECT OPERATION: ") | bold | color(Color::CyanLight),
                menu->Render() | color(Color::BlueLight),
            }) | border,
            filler(),
            text(">>> Use Arrows to navigate, Enter to select") | dim | hcenter,
        }) |
        border | center | size(WIDTH, EQUAL, 62);

    return to_center(box);
  });

  return CatchEvent(walletUI, [=](Event event) {
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

Component CLI::render_password_setup(void) {

  auto input_option = InputOption();
  input_option.multiline = false;
  input_option.password = true;
  auto pass_str = std::make_shared<std::string>(""); // FIX
  auto field = Input(pass_str.get(), "Enter password...", input_option);

  auto first_stage = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return && !pass_str->empty()) {
        bytes_data pass_str_in_bytes(pass_str->begin(), pass_str->end());
        actions->set_password_for_wallet(pass_str_in_bytes);

        // OPENSSL_cleanse(pass_str->data()), pass_str->size());
        OPENSSL_cleanse(pass_str_in_bytes.data(), pass_str_in_bytes.size());
        set_active_tab(CONFIRM_PASSWORD);

    }

    return false;
  });

  return Renderer(first_stage, [=, this] {
    auto box =
        vbox({text("CREATE PASSWORD") | bold | center | borderDouble |
                  color(Color::Cyan),
              separator(),
              vbox({
                  text("This password will encrypt your wallet on disk.") |
                      hcenter,
                  text("INPUT IS HIDDEN: No characters will be displayed.") |
                      hcenter | dim,
                  text("WARNING: If you forget it, your funds are LOST "
                       "FOREVER.") |
                      color(Color::Red) | hcenter | bold,
              }),

              separator(),
              hbox({
                  text(" >>> "),
                  field->Render() | border | size(WIDTH, EQUAL, 30),
              }),
              separator(),
              text(" Press [ENTER] to continue ") | hcenter | dim}) |
        border | size(WIDTH, EQUAL, 60);

    return to_center(box);
  });
}

Component CLI::render_confirm_password_setup(void) {
  auto input_option = InputOption();
  input_option.multiline = false;
  input_option.password = true;
  auto second_pass = std::make_shared<std::string>("");
  auto field = Input(second_pass.get(), "Enter password...", input_option);

  auto is_incorrect = std::make_shared<bool>(0);
  auto second_stage = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return) {
      bytes_data first_pass_in_bytes = actions->get_password_for_wallet();
      std::string first_pass(first_pass_in_bytes.begin(),
                             first_pass_in_bytes.end());

      // OPENSSL_cleanse(first_pass_in_bytes.data(),
      // first_pass_in_bytes.size());
      if (first_pass == *second_pass) {
        // OPENSSL_cleanse(first_pass.data(), first_pass.size());
        // OPENSSL_cleanse(second_pass.data(), second_pass.size());
        actions->save_wallet();
        set_active_tab(WALLET_UI);
        // NEXT STEP
      } else {
        *is_incorrect = true;
        second_pass->clear();
      }

      return true;
    }

    return false;
  });
  return Renderer(second_stage, [=, this] {
    std::vector<std::string> password_text = !*is_incorrect ? std::vector<std::string>{
    "CONFIRM MASTER PASSWORD",
    "Please re-enter your password to verify.",
    "If it doesn't match, you will have to restart.",
  } : std::vector<std::string> {
    "[!] ERROR: PASSWORDS DO NOT MATCH",
    "Please try again to ensure your funds are safe."
  };

    Color text_color = !*is_incorrect ? Color::Cyan : Color::Red;

    Elements elements;

    for (const auto &line : password_text) {
      elements.push_back(text(line) | hcenter);
    }

    auto box =
        vbox({vbox(std::move(elements)) | color(text_color) | bold, separator(),
              hbox({
                  filler(),
                  text(" >>> "),
                  field->Render() | border | size(WIDTH, EQUAL, 30),
                  filler(),
              }),

              separator(), text("Press [ENTER] to confirm") | dim | hcenter

        }) |
        border | size(WIDTH, EQUAL, 60);

    return to_center(box);
  });
}

Component CLI::render_request_unlock_password(void) {
  auto input_option = InputOption();
  auto attempts = std::make_shared<std::size_t>(0);
  constexpr static size_t max_attempts = 3;
  std::shared_ptr<std::string> user_input = std::make_shared<std::string>("");
  input_option.multiline = false;
  input_option.password = true;
  auto field = Input(user_input.get(), "Enter password...", input_option);

  auto component = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return && !user_input->empty()) {

      bytes_data password_in_bytes(user_input->begin(), user_input->end());
      if (actions->check_password(password_in_bytes)) {
        actions->load_wallet();
        set_active_tab(WALLET_UI);
      } else {
        *attempts += 1;
        if (*attempts == max_attempts) {
          tech_utils::rm_file();
          screen.Exit();
        }
      }
      return true;
    }

    return false;
  });

  return Renderer(component, [=, this] {
    int remaining = max_attempts - *attempts;
    Elements status_info;
    if (*attempts > 0) {
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
    auto box =
        vbox({text(" Unlock Wallet ") | bold | hcenter | borderDouble |
                  color(Color::Cyan),

              separator(),

              vbox({
                  text(" Please enter your password to decrypt the local "
                       "storage. ") |
                      hcenter,
                  text(" Your keys remain encrypted until a valid password "
                       "is provided. ") |
                      hcenter | dim,
              }),

              separator() | color(Color::DarkSlateGray1),

              vbox(std::move(status_info)) | hcenter,

              hbox({
                  text(" >>> ") | color(Color::Cyan),
                  field->Render() | border | flex | size(WIDTH, LESS_THAN, 70),
              }) | hcenter |
                  size(WIDTH, EQUAL, 60),

              separator(),

              text(" Press [ENTER] to unlock") | hcenter | dim

        }) |
        border | size(WIDTH, LESS_THAN, 70);

    return to_center(box);
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

void CLI::set_active_tab(int tab) {
  active_tab = tab;
  screen.Post(Event::Custom);
}

Element CLI::to_center(Element box) {
  return vbox({
      filler(),
      hbox({filler(), box, filler()}),
      filler(),
  });
}
