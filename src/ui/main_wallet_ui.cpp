#include "fmt/core.h"
#include "ui/cli.hpp"
#include "ui/ftxui-components/text_bytes.hpp"
#include "ui/ftxui-components/text_component.hpp"
#include "utils/tech_utils.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
Component CLI::print_wallet_ui(void) {
  static int selected = 0;
  static std::vector<std::string> entries =  {" 💸 SEND FUNDS     ", " 📜 HISTORY        ", " 🌐 NETWORK        ",
      " ➡  NEXT ADDR      ", " ⬅  PREV ADDR      ", " 🔑 EXPORT KEY     ",
      " 🚪 LOCK & EXIT    "};


       auto menu = Menu(&entries, &selected);

    auto menu_renderer = Renderer(menu, [=] {
      return menu->Render() | color(Color::CyanLight);
    });

  auto walletUI = Renderer(menu_renderer, [=, this] {
    WalletInfo wallet_info = actions->get_wallet();

    double balance_in_usd = tech_utils::eth_to_usd(
        wallet_info.balance, actions->get_current_eth_price());



      auto asset_panel = vbox({
        text(" 💰 ASSETS ") | bold | color(Color::Yellow),
        separatorDouble() | color(Color::Yellow),

        hbox({
          text(" ETH: ") | bold,
          text(fmt::format("{:.5f}", wallet_info.balance)) | color(Color::White) | bold
        }),

        hbox({
          text(" USD: ") | bold,
          text(fmt::format("{:.2f}", balance_in_usd)) | color(Color::Green)
        }),

        filler(),

        text(" ADDRESS: ") | dim,
        text_(wallet_info.addr) | color(Color::Cyan) | flex,
        text(" (Press 'C' to copy) ") | dim | hcenter,
      }) | borderHeavy | size(WIDTH, EQUAL, 38);

      float sync_progress = 0.85f; //!!!

      auto network_info = vbox({
        text(" 🌐 NETWORK & NODES ") | bold | color(Color::Magenta),
        separator(),

        hbox({
          text(" TARGET: "),
          text(actions->get_current_network()) | color(Color::Green)
        }),
        hbox({
          text(" GAS:    "),
          text("24 Gwei") | color(Color::Yellow)
        }),

        text(""),

        hbox({
           text(" SYNC PROGRESS: ") | dim,
           gauge(sync_progress) | color(Color::Cyan) | flex,
           text(fmt::format(" {:.0f} ", sync_progress * 100)) | dim
        })

      }) | borderHeavy;


      auto log_activity = vbox({
        text(" 📑 RECENT ACTIVITY ") | bold | dim,
        separator(),
        text(" • Connected to Infura/Alchemy node") | dim,
        text(" • Block 19283745 synced") | dim,
        text(" • Mempool monitor active") | dim
      }) | flex;

      auto footer = hbox({
        text(" [ENTER] EXECUTE ") | bold | hcenter | color(Color::Black) | bgcolor(Color::Cyan),
  
        text(" [C] COPY ADDRESS ") | dim,
        filler(),
        text(" [Q] LOCK ") | color(Color::RedLight),
        text("  Build: v1.0-alpha ") | dim
      });

      auto dashboard = vbox({
        hbox({
          text(" 💠 ETH-CORE WALLET v1.0 ") | bold | color(Color::Cyan),
          filler(),
          text(" SESSION: ACTIVE ") | color(Color::Green) | dim
        }),

        hbox({
          asset_panel,
          vbox({
            network_info,
            hbox({

          vbox({
            text(" AVAILABLE OPERATIONS ") | bold | color(Color::CyanLight),
            separator(),
            menu_renderer->Render()
          }) | borderHeavy | size(WIDTH, EQUAL, 30),
          log_activity | borderHeavy | flex
        }) | flex
      }) | flex
    }) | flex,
        footer

      });

   return to_center(dashboard | size(WIDTH, EQUAL, 100
) | size(HEIGHT, EQUAL, 28) | center);
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
    else if(event == Event::Character('q') || 
  event == Event::Character('Q')) {
    screen.Exit();
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
  menu_opts.on_enter = [=, this] { 
    actions->change_network(*selected); 
    set_active_tab(WALLET_UI);
  };

  const static std::vector<std::string> networks =
      networks::get_network_names();

  auto menu = Menu(&networks, selected.get(), menu_opts);

  auto component = Renderer(menu, [this, menu] {

    auto menu_render = menu->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 10) |
    color(Color::CyanLight);

    auto box =
        vbox({text(" 🌐 NETWORK SELECTION ") | bold | hcenter | color(Color::Cyan),
              separatorDouble() | color(Color::Cyan),

              text(" Select target provider: ") | dim | hcenter,
              text(""),
              menu_render,
              filler(), separatorLight(),

              hbox({text(" ACTIVE: ") | bold,
                    text(actions->get_current_network()) |
                        color(Color::GreenLight)}) |
                  hcenter,

              separatorLight(),

              text(" [ENTER] Select | [B] Back ") | hcenter | dim
              });


                

    return to_center(box | borderHeavy | size(WIDTH, EQUAL, 60) | size(HEIGHT, EQUAL, 16));
  });

  return CatchEvent(component, [this](Event event) {
    if (event == Event::Character('b') || event == Event::Character('B')) {
      set_active_tab(WALLET_UI);
      return true;
    }

    return false;
  });
}