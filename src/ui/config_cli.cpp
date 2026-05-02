#include "core/secure_bytes_data.hpp"
#include "ui/cli.hpp"
#include "utils/crypto_utils.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <openssl/crypto.h>
#include <string>

#include "ui/ftxui-components/input_component.hpp"

Component CLI::set_bit_length(void) {
  static int selected = 0;
  const static std::vector<std::string> entries = {"[1] 128 BITS (12 WORDS)",
                                                   "[2] 256 BITS (24 WORDS)"};

  const static std::vector<std::string> descriptions = {
      "Standard security. Ideal for Hot wallets and frequent transactions. "
      "Easy to write and store.",
      "Military-grade security. Recommended for \"Cold\" storage. Impossible "
      "to brute-force, even in theory."};
  auto menu = Menu(&entries, &selected);

  auto component = CatchEvent(menu, [this](Event event) {
    if (event == Event::Character('b') || event == Event::Character('B') ||
        event == Event::Escape) {
      set_active_tab(CONFIG_MENU);
      return true;
    } else if (event == Event::Return) {
      if (selected == 0) {
        actions->change_bit_length(128);
      }

      if (selected == 1) {
        actions->change_bit_length(256);
      }
      set_active_tab(CONFIG_MENU);
      return true;
    }
    return false;
  });

  return Renderer(component, [=, this] {
    auto description_element =
        paragraph(descriptions[selected]) | color(Color::GrayDark) | hcenter;
    auto box =
        vbox({

            text("SELECT ENTROPY LENGTH") | bold | hcenter |
                color(Color::Cyan2),
            separatorLight(),

            menu->Render() | hcenter | color(Color::White), separatorLight(),

            vbox({text("DESCRIPTION:") | dim | size(HEIGHT, ftxui::EQUAL, 1),
                  description_element}) |
                size(HEIGHT, EQUAL, 4),
            separatorLight(), text("[B] BACK TO CONFIG")}) |
        borderStyled(ROUNDED) | color(Color::Cyan2) | size(WIDTH, EQUAL, 60);

    return to_center(box);
  });
}

Component CLI::handle_extra_entropy(void) {
  static int selected = 0;
  const static std::vector<std::string> entries = {
      "[1] SYSTEM ONLY (OS CSPRNG)", "[2] SYSTEM + USER MIX (SHA-256 Hybrid)"};

  const static std::vector<std::string> descriptions = {
      "Standard mode. Uses /dev/urandom or BCryptGenRandom.Reliable and tested "
      "by millions of users.",
      "Paranoid mode. Mixes OS random with your secret input using SHA-256. "
      "Protects against OS backdoors."};
  auto active_sub_tab = std::make_shared<int>(0);
  auto menu = Menu(&entries, &selected);

  auto component = CatchEvent(menu, [=, this](Event event) {
    if (event == Event::Escape) {
      set_active_tab(CONFIG_MENU);
      return true;
    } else if (event == Event::Return) {
      if (selected == 0) {
        actions->set_extra_entropy(bytes_data{});
        set_active_tab(CONFIG_MENU);
      }

      if (selected == 1) {
        *active_sub_tab = 1;
      }

      return true;
    }
    return false;
  });

  auto user_input = std::make_shared<secure_string>("");

  auto input_option = InputOption();
  input_option.multiline = false;
  input_option.password = true;
  auto field = input_(*user_input);
  field->TakeFocus();

  auto text_box = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return) {
      actions->set_extra_entropy(
          bytes_data(user_input->begin(), user_input->end()));

      set_active_tab(CONFIG_MENU);
      *active_sub_tab = 0;
      return true;
    } else if (event == Event::Escape) {
      set_active_tab(CONFIG_MENU);
      return true;
    }
    return false;
  });
  auto container = Container::Tab({component, text_box}, active_sub_tab.get());
  return Renderer(container, [=, this] {
    Element box;
    if (*active_sub_tab == 1) {
      box = vbox(
          {text("ENTER EXTRA ENTROPY") | bold | hcenter | color(Color::Cyan3),
           separatorLight(), filler() | size(HEIGHT, EQUAL, 1),
           vbox({text_box->Render() | color(Color::White) | hcenter}) |
               borderStyled(DOUBLE) | color(Color::Cyan3),
           filler() | size(HEIGHT, EQUAL, 1),

           separatorLight(),

           text("Press [ENTER] to confirm | [ESC] to go back") | hcenter |
               dim});
    } else {
      auto description_element =
          paragraph(descriptions[selected]) | color(Color::GrayDark) | hcenter;
      box =
          vbox(
              {text("SELECT EXTRA ENTROPY SETTING") | bold | hcenter |
                   color(Color::Cyan2),
               separatorLight(),

               menu->Render(), separatorLight(),

               vbox({text("DESCRIPTION:") | dim | size(HEIGHT, ftxui::EQUAL, 1),
                     description_element}) |
                   size(HEIGHT, EQUAL, 4),
               separatorLight(),

               text("[B] BACK TO CONFIG")}) |
          borderStyled(ROUNDED) | color(Color::Cyan2) | size(WIDTH, EQUAL, 60);
    }
    return to_center(box);
  });
}

Component CLI::handle_passphrase(void) {
  static int selected = 0;
  const static std::vector<std::string> entries = {
      "[1] DISABLED (Standard Mode)", " [2] ENABLED (Advanced / 25th Word)"};

  const static std::vector<std::string> descriptions = {
      "Wallet depends ONLY on your 12/24 words.",
      "Adds a custom salt to your mnemonic. Creates a HIDDEN wallet. Requires "
      "absolute precision."};
  auto active_sub_tab = std::make_shared<int>(0);
  auto menu = Menu(&entries, &selected);

  auto component = CatchEvent(menu, [=, this](Event event) {
    if (event == Event::Character('b') || event == Event::Character('B') ||
        event == Event::Escape) {
      set_active_tab(CONFIG_MENU);
      return true;
    } else if (event == Event::Return) {
      if (selected == 0) {
        actions->add_passphrase({});
        set_active_tab(CONFIG_MENU);
      }

      if (selected == 1) {
        *active_sub_tab = 1;
      }

      return true;
    }
    return false;
  });

  auto user_input = std::make_shared<secure_string>("");

  auto input_option = InputOption();
  input_option.multiline = false;
  input_option.password = true;
  auto field = input_(*user_input);
  field->TakeFocus();

  auto text_box = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return) {
      actions->add_passphrase(std::move(*user_input));

      set_active_tab(CONFIG_MENU);
      *active_sub_tab = 0;
      return true;
    } else if (event == Event::Escape) {
      set_active_tab(CONFIG_MENU);
      return true;
    }
    return false;
  });
  auto container = Container::Tab({component, text_box}, active_sub_tab.get());
  return Renderer(container, [=, this] {
    Element box;
    if (*active_sub_tab == 1) {
      box =
          vbox({text("ENTER PASSPHRASE") | bold | hcenter | color(Color::Cyan3),
                separatorLight(), filler() | size(HEIGHT, EQUAL, 1),
                vbox({text_box->Render() | color(Color::White) | hcenter}) |
                    borderStyled(DOUBLE) | color(Color::Cyan3),
                filler() | size(HEIGHT, EQUAL, 1),

                separatorLight(),

                text("Press [ENTER] to confirm | [ESC] to go back") | hcenter |
                    dim});
    } else {
      auto description_element =
          paragraph(descriptions[selected]) | color(Color::GrayDark) | hcenter;
      box =
          vbox(
              {text("SELECT PASSPHRASE SETTING") | bold | hcenter |
                   color(Color::Cyan2),
               separatorLight(),

               menu->Render(), separatorLight(),
               vbox({text("DESCRIPTION:") | dim | size(HEIGHT, ftxui::EQUAL, 1),
                     description_element}) |
                   size(HEIGHT, EQUAL, 4),
               separatorLight(),

               text("[B] BACK TO CONFIG")}) |
          borderStyled(ROUNDED) | color(Color::Cyan2) | size(WIDTH, EQUAL, 60);
    }
    return to_center(box);
  });
}

Component CLI::handle_derivation_path(void) {
  static int selected = 0;
  const static std::vector<std::string> entries = {
      "[1] ETHEREUM STANDARD (BIP-44)", "[2] CUSTOM PATH (Advanced)"};

  const static std::vector<std::string> descriptions = {
      "m/44'/60'/0'/0/0 recommended for MetaMask, MyEtherWallet, and Ledger.",
      "Manually specify account, change, or address index.\n",
      "Use only if you know what you are doing!"};
  auto active_sub_tab = std::make_shared<int>(0);
  auto invalid_path = std::make_shared<bool>(0);
  auto menu = Menu(&entries, &selected);

  auto component = CatchEvent(menu, [=, this](Event event) {
    if (event == Event::Character('b') || event == Event::Character('B') ||
        event == Event::Escape) {
      set_active_tab(CONFIG_MENU);
      return true;
    } else if (event == Event::Return) {
      if (selected == 0) {
        actions->add_passphrase({});
        set_active_tab(CONFIG_MENU);
      }

      if (selected == 1) {
        *active_sub_tab = 1;
      }

      return true;
    } else if (event == Event::Escape) {
      set_active_tab(CONFIG_MENU);
      return true;
    }
    return false;
  });

  auto user_input = std::make_shared<secure_string>("");

  auto field = input_(*user_input);
  field->TakeFocus();

  auto text_box = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return) {
      if (!crypto_utils::is_valid_derive_path(std::string{*user_input})) {
        *invalid_path = true;
      } else {
        actions->change_derivation_path(std::move(*user_input));

        set_active_tab(CONFIG_MENU);
        *active_sub_tab = 0;
        *invalid_path = false;
      }
      return true;
    } else if(event == Event::Escape) {
      set_active_tab(CONFIG_MENU);
      return true;
    }

    return false;
  });
  auto container = Container::Tab({component, text_box}, active_sub_tab.get());
  return Renderer(container, [=, this] {
    Element box;
    if (*active_sub_tab == 1) {
      box = vbox(
          {text("ENTER DERIVATION PATH") | bold | hcenter | color(Color::Cyan3),
           separatorLight(), filler() | size(HEIGHT, EQUAL, 1),

           vbox({text_box->Render() | color(Color::White) | hcenter}) |
               borderStyled(DOUBLE) | color(Color::Cyan3),
           filler() | size(HEIGHT, EQUAL, 1),

           separatorLight(),

           text("Press [ENTER] to confirm | [ESC] to go back") | hcenter | dim

          });
    } else {
      auto description_element =
          paragraph(descriptions[selected]) | color(Color::GrayDark) | hcenter;
      box =
          vbox({
              text("SELECT PASSPHRASE SETTING") | bold | hcenter |
                  color(Color::Cyan2),
              separatorLight(),

              menu->Render(),
              separatorLight(),
              vbox({text("DESCRIPTION:") | dim | size(HEIGHT, ftxui::EQUAL, 1),
                    description_element}) |
                  size(HEIGHT, EQUAL, 4),
              separatorLight(),

              text("[B] BACK TO CONFIG"),
          }) |
          borderStyled(ROUNDED) | color(Color::Cyan2) | size(WIDTH, EQUAL, 60);
    }
    return to_center(box);
  });
}
