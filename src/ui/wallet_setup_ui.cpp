#include "ui/cli.hpp"
#include "ui/ftxui-components/paragraph.hpp"
#include "ui/ftxui-components/input_component.hpp"
#include "config/config.hpp"
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <string>
#include <utility>
#include <vector>
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
  input_option.multiline = true;

  input_option.on_change = [is_incorrect] {
    *is_incorrect = false;
  };


  auto field = input_(*user_input, false, input_option.on_change);
  field->TakeFocus();

  auto component = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return) {
      if (!user_input->empty()) {

        if (actions->check_mnemonic(*user_input)) {
          actions->set_mnemonic(std::move(*user_input));

          set_active_tab(ENTER_OPTIONAL_PASSPHRASE_MENU);
        } else
          *is_incorrect = true;
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
        text(" ⚠  VERIFICATION FAILED ") | bold | hcenter | color(Color::Red),

        vbox({

          text("• Check for typos or word order."),
          text("• Ensure words are from BIP-39 list."),
          text("• Check count (12/15/18/21/24).")
        }) | hcenter | dim,
 separatorLight() | color(Color::Red)
        }) | borderRounded | color(Color::Red);
         
      }

      auto input_box = vbox({

        hbox({
          text(" SEED PHRASE ") | bold | color(Color::Cyan),
          filler(),
          text(std::to_string(user_input->length()) + " chars") | dim
        }),

        separator(),

        text(""),

        hbox({
          text(" >>> ") | bold | color(Color::Yellow),
          field->Render()
        }),

        text("")

      }) | borderRounded | color(*is_incorrect ? Color::Red : Color::GrayDark);

      auto footer = vbox({
        text(" ESC: Cancel ") | dim,
        filler(),
        text(user_input->empty() ? "[ EMPTY ]" : "[ ENTER TO VERIFY ]") |
        color(user_input->empty() ? Color::GrayDark : Color::Green)
      });

      auto content = vbox({

        vbox({
          text(" 📥  IMPORT WALLET  ") | bold | hcenter | color(Color::Cyan),
          text(" Recovery phase is the master key to your funds ") | dim | hcenter
        }),

        filler(),

        error_box,
        filler(),
        input_box,
        filler(),
        footer

      });

      return to_center(content | borderDouble | size(WIDTH, EQUAL, 70) | size(HEIGHT, EQUAL, 22));
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
  auto entries = std::make_shared<std::vector<std::string>>();
  auto selected = std::make_shared<int>(0);

  auto target_tabs = std::make_shared<std::vector<state_t>>(std::vector<state_t>{
    EXTRA_ENTROPY_CONFIG, 
      BIT_LENGTH_CONFIG,
      PASSPHRASE_CONFIG, 
      DERIVE_PATH_CONFIG
  });
    
  auto update_entries = [entries, &cfg]() {
*entries  = {
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
  };

  update_entries();

  MenuOption menu_option;

  menu_option.entries_option.transform = [selected](const EntryState& state) {
    auto element = text(state.label);

    if(state.focused) {
      return hbox({
        text(" > "),
        element
      }) | inverted | color(Color::Yellow) | bold;
    }

    return hbox({
        text("   "),
        element
      }) | color(Color::GrayLight);
  };


  auto menu = Menu(entries.get(), selected.get(), menu_option);

  auto component = Renderer(menu, [=,&cfg, this] {
update_entries();

auto header = vbox({
  text(" ⚙  CONFIGURATION SETTINGS ") | bold | hcenter | color(Color::Cyan),
  text(" Adjust hardware-level entropy and derivation paths ") | dim | hcenter
});

auto footer = vbox({
  separator(),

  hbox({
    text(" [ENTER] Edit ") | dim,
    filler(),

        text(" [G] Generate ") | bold | color(Color::LightCyan1Bis),
        filler(),
              text(" [B] Back") | bold | color(Color::DarkCyan),
  })
}) | flex;

auto content = vbox({
  header,
  filler(),
  menu->Render() | borderRounded | color(Color::GrayDark),
  text(""),
  footer

});
    return to_center(content | borderDouble | size(WIDTH, GREATER_THAN, 100) | size(HEIGHT, GREATER_THAN, 20) | center);
  });
  return CatchEvent(component, [=, this](ftxui::Event event) {
    if (event == ftxui::Event::Return) {
      if(*selected < static_cast<int>(target_tabs->size())) {
        set_active_tab((*target_tabs)[*selected]);
      }

      return true;
    } else if (event == Event::Character('g') ||
               event == Event::Character('G')) {
      actions->create_wallet();
      return true;

    } else if (event == Event::Character('b') ||
               event == Event::Character('B') || event == Event::Escape) {
      set_active_tab(MAIN_MENU);
      return true;
    }
    return false;
  });
}