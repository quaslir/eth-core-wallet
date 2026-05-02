#include "fmt/core.h"
#include "ui/cli.hpp"
#include "ui/ftxui-components/text_bytes.hpp"
#include "ui/ftxui-components/text_component.hpp"
#include "utils/tech_utils.hpp"
#include <ftxui/dom/table.hpp>
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
                text(" ADDRESS: "),
                text_(wallet_info.addr),
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