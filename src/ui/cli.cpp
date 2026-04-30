#include "ui/cli.hpp"
#include "config/config.hpp"
#include "core/supported_networks.hpp"
#include "core/wallet_info.hpp"
#include "iwallet_actions.hpp"
#include "ui/ftxui-components/input_component.hpp"
#include "ui/ftxui-components/paragraph.hpp"
#include "ui/ftxui-components/text_bytes.hpp"
#include "ui/ftxui-components/text_component.hpp"
#include "utils/tech_utils.hpp"
#include <atomic>
#include <chrono>
#include <cstddef>

#include <fmt/core.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/color.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <thread>

void CLI::set_actions(IWalletActions *act) { actions = act; }

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
  auto extra_entropy_selection_config = handle_extra_entropy();
  auto passphrase_selection_config = handle_passphrase();
  auto derive_path_selection_config = handle_derivation_path();
  auto display_priv_key = display_private_key();
  auto trans_history = transaction_history_render();
  auto network_changer = change_network_render();
  auto root_container = Container::Tab(
      {main_menu, config_menu, import_wallet_ui, optional_passphrase,
       mnemonic_display, mnenonic_wiping_confirmation, set_password_component,
       confirm_password_component, wallet_ui, password_unlock,
       bit_length_selection_config, extra_entropy_selection_config,
       passphrase_selection_config, derive_path_selection_config,
       display_priv_key, trans_history, network_changer},
      &this->active_tab);

  std::atomic<bool> refresh_ui = true;
  std::thread refresh_thread([&] {
    while (refresh_ui) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      actions->update_balance();
      actions->update_transactions_data();
      actions->update_eth_price();
      screen.PostEvent(Event::Custom);
    }
  });
  screen.Loop(root_container);

  refresh_ui = false;
  if (refresh_thread.joinable()) {
    refresh_thread.join();
  }
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
  auto mnemonic = std::make_shared<bytes_data>(actions->get_mnemonic());
  auto button = Button(
      " [ PRESS ENTER TO CONTINUE ] ",
      [this, mnemonic] {
        if (!mnemonic->empty()) {
          tech_utils::clear(*mnemonic);
          mnemonic->clear();
        }

        this->set_active_tab(MNEMONIC_WIPING);
      },
      ButtonOption::Ascii());
  return Renderer(button, [this, button, mnemonic] {
    auto box = vbox({

                   vbox({
                       text(" YOUR RECOVERY PHRASE (SEED) ") | bold | center,

                   }) | borderDouble |
                       color(Color::Yellow),

                   separator(),

                   paragraph_(actions->get_mnemonic()) | hcenter |
                       color(Color::White) | bold,

                   separator(),

                   vbox({
                       text(" WARNING: DO NOT SCREENSHOT! ") | center,
                       text(" WRITE IT DOWN ON PAPER NOW! ") | center,
                   }) | border |
                       color(Color::Red),

                   separator(), button->Render() | hcenter | bold | focus

               }) |
               border | center | size(WIDTH, EQUAL, 60) |
               size(HEIGHT, EQUAL, 15);

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
  auto user_input = std::make_shared<bytes_data>();

  auto input_option = InputOption();
  input_option.multiline = false;

  auto field = input_(*user_input);
  field->TakeFocus();

  auto component = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return) {
      if (!user_input->empty()) {
        std::string_view view_mnemonic{
            reinterpret_cast<const char *>(user_input->data()),
            user_input->size()};
        if (actions->check_mnemonic(view_mnemonic)) {
          actions->set_mnemonic(view_mnemonic);
          tech_utils::clear(*user_input);
          user_input->clear();
          set_active_tab(ENTER_OPTIONAL_PASSPHRASE_MENU);
        } else
          *is_incorrect = true;
        user_input->clear();
        return true;
      } else
        *is_incorrect = true;
      user_input->clear();
      return true;
    } else if (event == Event::Escape) {
      set_active_tab(MAIN_MENU);
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
                text("(Press ESC to return to main menu)") |
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
  auto user_input = std::make_shared<bytes_data>();

  auto input_option = InputOption();
  input_option.multiline = false;
  input_option.password = true;

  auto field = input_(*user_input, true);
  field->TakeFocus();

  auto component = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return) {
      actions->set_passphrase(
          std::string_view{reinterpret_cast<const char *>(user_input->data()),
                           user_input->size()});

      tech_utils::clear(*user_input);

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
    };

    auto box =
        vbox({
            text(" CONFIGURATION SETTINGS ") | bold | hcenter |
                color(Color::Cyan),
            separator(),
            vbox({[]() {
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

            text(" Use Arrows to navigate, Enter to toggle/select") | dim |
                hcenter,
            separator(),
            text(" [B] Back to Main Menu") | bold | color(Color::DarkCyan),
            text(" [G] Generate Wallet") | bold | color(Color::LightCyan1Bis),

        }) |
        center;

    return to_center(box);
  });
  return CatchEvent(component, [&](ftxui::Event event) {
    if (event == ftxui::Event::Return) {
      switch (selected) {
      case 0:
        set_active_tab(EXTRA_ENTROPY_CONFIG);
        break;
      case 1:
        set_active_tab(BIT_LENGTH_CONFIG);
        break;
      case 2:

        set_active_tab(PASSPHRASE_CONFIG);
        break;
      case 3:
        set_active_tab(DERIVE_PATH_CONFIG);
        break;
      case 4:
        set_active_tab(MAIN_MENU);
        break;
      }

      return true;
    } else if (event == Event::Character('g') ||
               event == Event::Character('G')) {
      actions->create_wallet();
      return true;

    } else if (event == Event::Character('b') ||
               event == Event::Character('B')) {
      set_active_tab(MAIN_MENU);
      return true;
    }
    return false;
  });
}

Component CLI::print_wallet_ui(void) {
  static int selected = 0;
  static std::vector<std::string> entries;
  auto menu = Menu(&entries, &selected);
  entries = {" 1. Send Transaction ", " 2. Transaction history",
             " 3. Change Network",    " 4. Next Address",
             " 5. Previous Address",  " 6. Export Key",
             " 7. Lock & Exit"};

  auto walletUI = Renderer(menu, [=, this] {
    WalletInfo wallet_info = actions->get_wallet();

    double balance_in_usd = tech_utils::eth_to_usd(
        wallet_info.balance, actions->get_current_eth_price());

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
                info_line(" BALANCE: ",
                          fmt::format("{:.5f}", wallet_info.balance) + " ETH " +
                              "~ " + fmt::format("{:.2f}", balance_in_usd) +
                              " USD",
                          Color::Yellow),
                info_line(" ADDRESS: ", wallet_info.addr,
                          Color::DarkSlateGray1),
                info_line(" NETWORK: ", actions->get_current_network(),
                          Color::Green),

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

  return CatchEvent(walletUI, [=, this](Event event) {
    if (event == Event::Return) {
      actions->apply_choice_from_wallet_ui(selected + 1);
      return true;

    } else if (event == Event::Character('c') ||
               event == Event::Character('C')) {
      actions->copy_address();
      return true;
    }

    return false;
  });
}

Component CLI::render_password_setup(void) {

  auto input_option = InputOption();
  input_option.multiline = false;
  input_option.password = true;
  auto pass_str = std::make_shared<bytes_data>();
  auto field = input_(*pass_str, true);

  auto first_stage = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return && !pass_str->empty()) {
      actions->set_password_for_wallet(*pass_str);

      tech_utils::clear(*pass_str);
      pass_str->clear();
      set_active_tab(CONFIRM_PASSWORD);
      return true;
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

  auto second_pass = std::make_shared<bytes_data>();
  auto field = input_(*second_pass, true);

  auto is_incorrect = std::make_shared<bool>(0);
  auto second_stage = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return) {
      auto first_pass =
          std::make_shared<bytes_data>(actions->get_password_for_wallet());

      if (*first_pass == *second_pass) {
        tech_utils::clear(*first_pass);
        tech_utils::clear(*second_pass);
        first_pass->clear();
        second_pass->clear();
        actions->save_wallet();
        actions->update_balance();
        set_active_tab(WALLET_UI);
      } else {
        second_pass->clear();
        *is_incorrect = true;
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
  auto user_input = std::make_shared<bytes_data>();
  input_option.multiline = false;
  input_option.password = true;
  auto field = input_(*user_input, true);

  auto component = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return && !user_input->empty()) {

      if (actions->check_password(*user_input)) {
        tech_utils::clear(*user_input);
        user_input->clear();
        actions->load_wallet();
        actions->update_balance();
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

Component CLI::display_private_key(void) {
  auto active_sub_tab = std::make_shared<int>(0);

  auto button_subtab_0 = Button(
      "Press [ENTER] to reveal | [ESC] to cancel", [&] {},
      ButtonOption::Ascii());

  auto button_subtab_1 =
      Button("Press [ENTER] to close the menu", [&] {}, ButtonOption::Ascii());

  Component warning_view = Renderer(button_subtab_0, [=, this] {
    auto element =
        vbox({text("⚠️  CRITICAL SECURITY WARNING  ⚠️  ") | bold | hcenter |
                  color(Color::Red1),
              separatorDouble() | color(Color::Red1),

              paragraph("You are about to reveal your PRIVATE KEY. Anyone who "
                        "sees this key can take full control of your funds "
                        "forever. Do not share it with anyone, including "
                        "support staff or developers.") |
                  hcenter,
              filler() | size(HEIGHT, EQUAL, 1),

              vbox({text(" • NEVER share this key via email or chat.") |
                        color(Color::Yellow),
                    text(" • NEVER enter it on websites you don't trust.") |
                        color(Color::Yellow),
                    text(" • ALWAYS store it in a physical, offline place.") |
                        color(Color::Green)}) |
                  hcenter,

              filler() | size(HEIGHT, EQUAL, 1),

              button_subtab_0->Render() | hcenter | blink |
                  color(Color::Red1)}) |
        borderDouble | color(Color::Red1) | bgcolor(Color::Black) |
        size(WIDTH, EQUAL, 60);

    return to_center(element);
  });

  warning_view->TakeFocus();
  auto warning_component = CatchEvent(warning_view, [=, this](Event event) {
    if (event == Event::Return) {
      *active_sub_tab = 1;
      return true;
    } else if (event == Event::Escape) {
      set_active_tab(WALLET_UI);
      return true;
    }

    return false;
  });

  Component private_key_view = Renderer(button_subtab_1, [=, this] {
    const bytes_data &private_key_in_bytes = actions->get_private_key();

    auto content_box =
        vbox(
            {text(" YOUR PRIVATE KEY ") | bold | hcenter | color(Color::Yellow),

             separatorLight(), filler(),

             byte_text(private_key_in_bytes) | hcenter | color(Color::Red1),

             filler(),

             separatorLight(),
             text("Press C to copy private key") | bold | color(Color::Yellow),
             button_subtab_1->Render() | dim | hcenter}) |
        borderStyled(ROUNDED) | color(Color::Red1) | size(WIDTH, EQUAL, 60) |
        size(HEIGHT, EQUAL, 10) | hcenter;

    return to_center(content_box);
  });

  auto private_key_displayer_component =
      CatchEvent(private_key_view, [=, this](Event event) {
        if (event == Event::Return) {
          *active_sub_tab = 0;
          set_active_tab(WALLET_UI);
          return true;
        } else if (event == Event::Character('c') ||
                   event == Event::Character('C')) {
          actions->copy_private_key();
        }

        return false;
      });
  auto container =
      Container::Tab({warning_component, private_key_displayer_component},
                     active_sub_tab.get());

  return Renderer(container, [=] {
    container->ChildAt(*active_sub_tab)->TakeFocus();
    if (*active_sub_tab == 0) {
      return warning_component->Render();
    }
    return private_key_displayer_component->Render();
  });
}

Component CLI::transaction_history_render(void) {

  const auto buttons = Container::Horizontal(
      {Button(
           " [B] BACK ", [&] { set_active_tab(WALLET_UI); },
           create_button("[B] BACK", Color::Red)),
       Button(
           " [R] REFRESH ", [&] { actions->request_transactions_data(); },
           create_button("[R] REFRESH", Color::Cyan2))});

  auto component = Renderer(buttons, [buttons, this]() mutable -> Element {
    auto history = actions->get_transactions_history();
    if (history.empty()) {
      return vbox({text("LOADING...") | bold | hcenter | color(Color::Cyan2)}) |
             center;
    }

    std::vector<std::vector<std::string>> table_data;
    table_data.push_back(
        {" DATE ", " TYPE ", " AMOUNT ", " FROM ", " TO ", " HASH "});
    for (const auto &tx : history) {
      table_data.push_back(
          {tx.timestamp.substr(5, 11), tx.incoming ? " IN " : " OUT ",
           std::to_string(tx.value).substr(0, 6) + " " + tx.asset,
           tx.from.substr(0, 6) + "..." + tx.from.substr(38),
           tx.to.substr(0, 6) + "..." + tx.to.substr(38),
           tx.hash.substr(0, 8) + "..."});
    }

    auto table = Table(table_data);

    table.SelectRow(0).Decorate(bold | bgcolor(Color::Blue) |
                                color(Color::White));
    table.SelectRow(0).SeparatorVertical();

    table.SelectColumn(0).Decorate(color(Color::GrayDark));
    table.SelectColumn(2).Decorate(color(Color::GreenLight) | bold);

    table.SelectAll().SeparatorVertical(LIGHT);
    table.SelectAll().Border(LIGHT);
    auto box = vbox({text(" TRANSACTION HISTORY ") | bold | hcenter |
                         color(Color::Yellow),
                     separator(), table.Render() | frame | hcenter | flex,

                     hbox({buttons->Render() | hcenter}) | hcenter

               }) |
               borderDouble | color(Color::BlueLight) | flex |
               size(HEIGHT, EQUAL, 20);

    return to_center(box);
  });

  return CatchEvent(component, [this](Event event) {
    if (event == Event::Character('b') || event == Event::Character('B')) {
      set_active_tab(WALLET_UI);
      return true;
    } else if (event == Event::Character('r') ||
               event == Event::Character('R')) {
      actions->request_transactions_data();
      return true;
    }
    return false;
  });
}

Component CLI::change_network_render(void) {

  auto menu_opts = MenuOption::Vertical();
  auto selected = std::make_shared<int>(0);
  menu_opts.on_enter = [=, this] { actions->change_network(*selected); };

  const static std::vector<std::string> networks =
      networks::get_network_names();

  auto menu = Menu(&networks, selected.get(), menu_opts);

  auto component = Renderer(menu, [this, menu] {
    auto box =
        vbox({text(" 🌐 SELECT NETWORK ") | bold | hcenter | color(Color::Cyan),
              separatorDouble(),
              menu->Render() | frame | size(HEIGHT, ftxui::LESS_THAN, 10) |
                  hcenter,
              filler(), separatorLight(),

              hbox({text(" CURRENT: ") | bold,
                    text(actions->get_current_network()) |
                        color(Color::GreenLight)}) |
                  hcenter,

              separatorLight(),

              hbox({text(" Press "), text("[B]") | bold | color(Color::Yellow),
                    text(" to return to main menu")}) |
                  hcenter | color(Color::GrayLight)}) |
        borderDouble | center | size(WIDTH, ftxui::EQUAL, 60);

    return to_center(box);
  });

  return CatchEvent(component, [this](Event event) {
    if (event == Event::Character('b') || event == Event::Character('B')) {
      set_active_tab(WALLET_UI);
      return true;
    }

    return false;
  });
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

int CLI::get_tab_data(void) const { return active_tab; }

ButtonOption CLI::create_button(const std::string &label, Color c) const {
  auto opt = ButtonOption::Ascii();

  opt.transform = [label, c](const EntryState &state) {
    auto t = text(" " + label + " ");
    return state.focused ? (t | inverted | color(c)) : (t | color(c));
  };

  return opt;
}
