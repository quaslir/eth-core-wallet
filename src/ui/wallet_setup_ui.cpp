#include "ui/cli.hpp"
#include "ui/ftxui-components/paragraph.hpp"
#include "ui/ftxui-components/input_component.hpp"
#include "config/config.hpp"
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <utility>
Component CLI::create_main_menu(void) {

  static std::vector<std::string> entries = {" ⚡ GENERATE NEW WALLET ",
      " 📂 IMPORT EXISTING    ",
      " ❌ EXIT                "};
  static int selected = 0;

    MenuOption menu_option;

    menu_option.entries_option.transform = [](const EntryState& state) {
      auto t = text(state.label);

      if(state.focused) {
        return t | bold | color(Color::Cyan) | inverted;
      }

      return t | dim;
    };  

  auto menu = Menu(&entries, &selected, menu_option);

  auto component = ftxui::CatchEvent(menu, [&](ftxui::Event event) {
    if (event == ftxui::Event::Character('q') ||
        event == ftxui::Event::Character('Q')) {
      actions->on_main_menu(2);
      return true;
    } else if(event == Event::Return) {
      actions->on_main_menu(selected);
    }
    return false;
  });

  return Renderer(component, [=, this] {
    auto header = vbox({
        text(" ╔══════════════════════════════════════════╗ ") | hcenter | color(Color::Cyan),
        text(" ║        ETH CORE WALLET v1.0 [PRO]        ║ ") | bold | hcenter | color(Color::Cyan),
        text(" ╚══════════════════════════════════════════╝ ") | hcenter | color(Color::Cyan),
    });


    auto menu_render = vbox({
        text(" MAIN OPERATIONS ") | hcenter | bold,
        separator(),
        menu->Render() | center,
    }) | borderRounded | color(Color::GrayLight) | size(WIDTH, EQUAL, 80) | hcenter;

    auto info = hbox({
      text(" [STATUS]: ") | dim,
      text("READY") | color(Color::Green) | bold,
      filler(),

    }) | borderLight;

    auto footer = vbox({
      text(" Q: Quit ") | dim,
        filler(),
        text(" Arrows: Navigate | Enter: Select ") | dim
    });


    return to_center(
      vbox({
        header,
        filler(),
        menu_render,
        filler(), 
        info,
        footer
      }) | borderDouble | size(HEIGHT, LESS_THAN, 25) | size(WIDTH, LESS_THAN, 90)
    );


  });
}

Component CLI::render_mnemonic_element(void) {
  auto mnemonic = std::make_shared<secure_string>(actions->get_mnemonic());


  auto button = Button(
      " [ I HAVE WRITTEN IT DOWN ] ",
      [this, mnemonic] { this->set_active_tab(MNEMONIC_WIPING); },
      ButtonOption::Ascii());

  return Renderer(button, [this, button, mnemonic] {

    if(mnemonic->empty()) {
      *mnemonic = actions->get_mnemonic();
    }

    auto header = vbox({
      text(" 🔐 SECURITY: RECOVERY PHRASE ") | bold | hcenter | color(Color::Yellow),
      text(" Keep this phrase private and offline! ") | dim | hcenter
    });

    auto phrase_display = vbox({
      filler(),
      paragraph_(*mnemonic) | bold | hcenter | color(Color::White),
      filler()
    }) | borderRounded | color(Color::Cyan) | size(HEIGHT, EQUAL, 6);

    auto warning_box = vbox({
        text(" ⚠️  CRITICAL WARNING  ⚠️ ") | bold | hcenter,
        separator(),
        text(" • DO NOT take a screenshot or digital copy.") | hcenter,
        text(" • Anyone with these words can STEAL your money.") | hcenter,
          text(" • There is NO 'Forgot Phrase' option.") | hcenter
    }) | borderDouble | color(Color::Red);

    auto content = vbox({
      header,
      filler(),
      phrase_display,
      filler(),
      warning_box,
      filler(),
      button->Render() | hcenter | bold | focus | color(Color::Green)
    });

    return to_center(content | border | size(WIDTH, EQUAL, 70) | size(HEIGHT, EQUAL, 22));

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
      actions->wipe_mnemonic();
      std::cerr << actions->get_mnemonic().c_str() << std::endl;
      set_active_tab(SET_PASSWORD);
    }
  };

  auto field = Input(user_input.get(), "Type here...", input_option);
  field->TakeFocus();

  return Renderer(field, [=, this] {
    bool is_correct = (*user_input == "I AM RESPONSIBLE");
    auto input_style = is_correct ? color(Color::Green) : color(Color::Red);

    auto warning_header = vbox({
      text(" ⚠️  LEGAL & SECURITY TERMINATION  ⚠️ ") | bold | hcenter
    }) | borderDouble | color(Color::Red);

    auto warning_list = vbox({
      text(" 1. This app will NOW WIPE the mnemonic from memory.") | color(Color::GrayLight),
      
      text(" 2. We have ZERO copies. No database, no logs, no backups.") |  color(Color::GrayLight),
      text(" 3. If you didn't write it down, your funds are ALREADY LOST.") |  color(Color::Red) | bold,
      text("")
    }) | hcenter;
    

    auto input_box = vbox({
      text(" To proceed, type exactly: ") | hcenter | dim,
      text(""),
      text(" I AM RESPONSIBLE ") | bold | hcenter | inverted | color(Color::Yellow),

      separator(),

      hbox({
        text(" >>> "),
        field->Render() | size(WIDTH, EQUAL, 30)
      }) | hcenter
    }) | borderRounded | color(is_correct ? Color::Green : Color::GrayDark);

    auto status_msg = is_correct ? text(" [ READY TO WIPE: PRESS ENTER ] ") | bold | color(Color::Green) :
    text(" [ WAITING FOR CORRECT INPUT ] ") | dim | hcenter;

    auto content = vbox({
      warning_header,
      warning_list,
      filler(),
      input_box,
      filler(),
            text(""),
      status_msg | hcenter,

    });

    return to_center(content) | border | size(WIDTH, EQUAL, 70) | size(HEIGHT, EQUAL, 22) | center;

   
  });
}

Component CLI::render_import_mnemonic_component(void) {
  auto is_incorrect = std::make_shared<bool>(0);
  auto user_input = std::make_shared<secure_string>();

  auto input_option = InputOption();
  input_option.multiline = false;

  auto field = input_(*user_input);
  field->TakeFocus();

  auto component = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return) {
      if (!user_input->empty()) {

        if (actions->check_mnemonic(*user_input)) {
          actions->set_mnemonic(std::move(*user_input));

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
  auto user_input = std::make_shared<secure_string>();

  auto input_option = InputOption();
  input_option.multiline = false;
  input_option.password = true;

  auto field = input_(*user_input, true);
  field->TakeFocus();

  auto component = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return) {
      actions->set_passphrase(std::move(*user_input));

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
        " 4. DERIVATION     : [ " + std::string{cfg.derivation_path} + " ]",
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