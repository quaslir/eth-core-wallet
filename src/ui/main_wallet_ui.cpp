#include "core/asset.hpp"
#include "core/wallet_info.hpp"
#include "drivers/tx_status_client.hpp"
#include "fmt/core.h"
#include "ui/cli.hpp"
#include "ui/ftxui-components/text_bytes.hpp"
#include "ui/ftxui-components/text_component.hpp"
#include "utils/tech_utils.hpp"
#include <cstdint>
#include <fmt/chrono.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/color.hpp>
#include <memory>
#include <string>
Component CLI::print_wallet_ui(void) {
  static int selected = 0;
  static std::vector<std::string> entries = {
      " 💸 SEND FUNDS     ", " 📜 HISTORY        ", " 🌐 NETWORK        ",
      " ➡  NEXT ADDR      ", " ⬅  PREV ADDR      ", " 🔑 EXPORT KEY     ",
      " 🚪 LOCK & EXIT    "};

  auto menu = Menu(&entries, &selected);

  auto menu_renderer =
      Renderer(menu, [=] { return menu->Render() | color(Color::CyanLight); });

  auto walletUI = Renderer(menu_renderer, [=, this] {
    WalletInfo wallet_info = actions->get_wallet();
    Elements asset_rows;
    for (auto const &[id, asset] : *wallet_info.assets) {
      asset_rows.push_back(hbox(
          {text(" " + asset.symbol + ": ") | bold | size(WIDTH, EQUAL, 8),
           text(fmt::format("{:.5f}", asset.balance)) | color(Color::White),
           filler(),
           text(fmt::format("{:.2f}", asset.balance * asset.fiat_price)) |
               color(Color::Green)}));
    }
    asset_rows.push_back(separator());
    asset_rows.push_back(

        hbox({text(" TOTAL PORTFOLIO: ") | bold | color(Color::Yellow),
              filler(),
              text(fmt::format("{:.2f} USD", wallet_info.total)) | bold |
                  color(Color::Green)}));

    auto asset_panel = vbox({
                           text(" 💰 ASSETS ") | bold | color(Color::Yellow),
                           separatorDouble() | color(Color::Yellow),

                           vbox(std::move(asset_rows)),

                           separator(),

                           text(" ADDRESS: ") | dim,
                           text_(wallet_info.addr) | color(Color::Cyan) | flex,
                           text(" (Press 'C' to copy) ") | dim | hcenter,
                       }) |
                       borderHeavy | size(WIDTH, EQUAL, 45);

    float refresh_in = actions->get_next_refresh();

    auto network_info =
        vbox({text(" 🌐 NETWORK & NODES ") | bold | color(Color::Magenta),
              separator(),

              hbox({text(" TARGET: "), text(actions->get_current_network()) |
                                           color(Color::Green)}),
              hbox({text(" GAS:    "),
                    text(fmt::format("{:.3f}",
                                     actions->get_current_gas_price().first) +
                         " GWei") |
                        color(Color::Yellow)}),

              text(""),

              hbox({text(" REFRESH IN: ") | dim,
                    gauge(refresh_in) | color(Color::Blue) | flex,
                    text(fmt::format(" {:.0f} ", refresh_in * 100)) | dim})

        }) |
        borderHeavy;
    Elements log = {
        text(" 📑 RECENT ACTIVITY ") | bold | dim,
        separator(),
    };

    const auto &activity = actions->get_activity();

    if (activity.empty()) {
      log.push_back(text(" No recent activity ") | dim);
    } else {
      for (const auto &event : activity) {
        log.push_back(
            vbox({text(" " + event.icon + " " + event.msg), filler(),
                  text(fmt::format(
                      "{:%H:%M:%S}",
                      fmt::localtime(
                          std::chrono::system_clock::to_time_t(event.time)))) |
                      dim}));
      }
    }

    auto footer = hbox({text(" [ENTER] EXECUTE ") | bold | hcenter |
                            color(Color::Black) | bgcolor(Color::Cyan),

                        text(" [C] COPY ADDRESS ") | dim, filler(),
                        text(" [Q] LOCK ") | color(Color::RedLight),
                        text("  Build: v1.0-alpha ") | dim});

    auto dashboard = vbox(
        {hbox({text(" 💠 ETH-CORE WALLET v1.0 ") | bold | color(Color::Cyan),
               filler(),
               text(" SESSION: ACTIVE ") | color(Color::Green) | dim}),

         hbox({asset_panel,
               vbox({network_info,
                     hbox({

                         vbox({text(" AVAILABLE OPERATIONS ") | bold |
                                   color(Color::CyanLight),
                               separator(), menu_renderer->Render()}) |
                             borderHeavy | size(WIDTH, EQUAL, 35),
                         vbox(log) | borderHeavy | size(WIDTH, EQUAL, 50)}) |
                         flex}) |
                   flex}) |
             flex,
         footer

        });

    return to_center(dashboard | size(WIDTH, EQUAL, 120) |
                     size(HEIGHT, EQUAL, 34) | center);
  });

  return CatchEvent(walletUI, [=, this](Event event) {
    if (event == Event::Return) {
      actions->apply_choice_from_wallet_ui(selected + 1);
      return true;

    } else if (event == Event::Character('c') ||
               event == Event::Character('C')) {
      actions->copy_address();
      return true;
    } else if (event == Event::Character('q') ||
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
           " [R] REFRESH ", [&] { actions->update_transactions_data(); },
           create_button("[R] REFRESH", Color::Cyan2))});

  auto component = Renderer(buttons, [buttons, this]() mutable -> Element {
    auto [history, error] = actions->get_transactions_history();
    if (history.empty() && !error) {
      return vbox({text("LOADING...") | bold | hcenter | color(Color::Cyan2)}) |
             center;
    } else if (error) {
      return vbox({text("Could not load data") | bold | hcenter |
                   color(Color::Red1)}) |
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
    actions->update_balance(true);
    actions->update_gas_price(true);
    set_active_tab(WALLET_UI);
  };

  const static std::vector<std::string> networks =
      networks::get_network_names();

  auto menu = Menu(&networks, selected.get(), menu_opts);

  auto component = Renderer(menu, [this, menu] {
    auto menu_render = menu->Render() | vscroll_indicator | frame |
                       size(HEIGHT, LESS_THAN, 10) | color(Color::CyanLight);

    auto box = vbox(
        {text(" 🌐 NETWORK SELECTION ") | bold | hcenter | color(Color::Cyan),
         separatorDouble() | color(Color::Cyan),

         text(" Select target provider: ") | dim | hcenter, text(""),
         menu_render, filler(), separatorLight(),

         hbox({text(" ACTIVE: ") | bold, text(actions->get_current_network()) |
                                             color(Color::GreenLight)}) |
             hcenter,

         separatorLight(),

         text(" [ENTER] Select | [B] Back ") | hcenter | dim});

    return to_center(box | borderHeavy | size(WIDTH, EQUAL, 60) |
                     size(HEIGHT, EQUAL, 16));
  });

  return CatchEvent(component, [this](Event event) {
    if (event == Event::Character('b') || event == Event::Character('B')) {
      set_active_tab(WALLET_UI);
      return true;
    }

    return false;
  });
}

Component CLI::make_transaction_render(void) {
  auto to_addr = std::make_shared<std::string>();
  auto amount_str = std::make_shared<std::string>();
  auto selected_asset = std::make_shared<int>(0);
  auto error_msg = std::make_shared<std::string>();
  auto success_msg = std::make_shared<std::string>();
  auto gas_multiplayer = std::make_shared<int>(0);
  auto final_gas = std::make_shared<double>(0.0);
  auto selected_subtab = std::make_shared<int>(0);
  auto preview_addr = std::make_shared<std::string>();
  auto current_asset = std::make_shared<Asset>();
  auto tx_status = std::make_shared<TxStatus>(TxStatus::ERROR);
  auto sending = std::make_shared<bool>(0);
  auto spin_idx = std::make_shared<int>(0);
  auto custom_gas_limit = std::make_shared<std::string>();
  auto custom_gas = std::make_shared<std::string>();
  auto spinner =
      std::make_shared<std::vector<std::string>>(std::vector<std::string>{
          "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"});
  static const std::vector<std::string> gas_options = {
      " 🐢 SLOW ", " ⚡ NORMAL ", " 🚀 FAST ", " ✏️  CUSTOM "};
  InputOption input_opt;
  input_opt.multiline = false;
  input_opt.on_change = [=]() {
    error_msg->clear();
    success_msg->clear();
  };
  auto addr_input = Input(to_addr.get(), "0x...", input_opt);
  auto amount_input = Input(amount_str.get(), "0.00", input_opt);
  auto gas_limit_input = Input(custom_gas_limit.get(), "auto", input_opt);
  auto asset_names = std::make_shared<std::vector<std::string>>();
  auto custom_gas_input = Input(custom_gas.get(), "0.001", input_opt);

  auto asset_toggle = Toggle(asset_names.get(), selected_asset.get());

  auto gas_toggle = Toggle(&gas_options, gas_multiplayer.get());

  auto container_send =
      Container::Vertical({asset_toggle, addr_input, amount_input, gas_toggle,
                           gas_limit_input, custom_gas_input});

  auto send_form = Renderer(container_send, [=, this]() {
    asset_names->clear();

    WalletInfo info = actions->get_wallet();

    for (const auto &[id, asset] : *info.assets) {
      asset_names->push_back(" " + asset.symbol + " ");
    }
    int idx = 0;
    for (const auto &[id, asset] : *info.assets) {
      if (idx == *selected_asset) {
        *current_asset = asset;
        break;
      }
      idx++;
    }

    double gas_price = actions->get_current_gas_price().first;
    std::array<double, 3> multiplayer = {0.8, 1.0, 1.5};
    if (*gas_multiplayer == 3) {
      double custom_val = 0.0;
      if (!custom_gas->empty() &&
          tech_utils::to_double(*custom_gas, custom_val)) {
        *final_gas = custom_val;
      } else
        *final_gas = gas_price;
    } else {
      *final_gas = gas_price * multiplayer[*gas_multiplayer];
    }
    *preview_addr = to_addr->empty() ? "--"
                    : (to_addr->size() >= 10)
                        ? to_addr->substr(0, 6) + "..." +
                              to_addr->substr(to_addr->size() - 4)
                        : *to_addr;

    bool addr_valid = to_addr->size() == 42 && to_addr->substr(0, 2) == "0x";
    bool amount_valid = !amount_str->empty();

    auto box = vbox(
        {text(" 💸 SEND FUNDS ") | bold | color(Color::Cyan) | hcenter,
         separatorDouble() | color(Color::Cyan),

         hbox({text(" ASSET:  ") | dim,
               asset_toggle->Render() | color(Color::CyanLight)}),

         hbox({
             text(" BALANCE: ") | dim,
             filler(),
             text(fmt::format("{:.5f} {}", current_asset->balance,
                              current_asset->symbol)) |
                 color(Color::Green),
         }),

         separator(),

         hbox(
             {text(" TO:     ") | dim,
              addr_input->Render() | flex |
                  (addr_valid ? color(Color::White) : color(Color::RedLight))}),

         hbox({text(" AMOUNT: ") | dim, amount_input->Render() | flex,
               text(" " + current_asset->symbol + " ") | dim}),

         hbox({text(" GAS:    ") | dim,
               gas_toggle->Render() | color(Color::CyanLight), filler(),
               text(fmt::format("{:.4f} Gwei", *final_gas)) |
                   color(Color::YellowLight)}),

         *gas_multiplayer == 3
             ? hbox({text(" CUSTOM: ") | dim, custom_gas_input->Render() | flex,
                     text(" Gwei ") | dim})
             : text(""),

         hbox({text(" LIMIT ") | dim, gas_limit_input->Render() | flex,
               text(" units ") | dim

         }),

         separator(),
         error_msg->empty()
             ? text("")
             : text(" ✗ " + *error_msg) | color(Color::Red1) | hcenter,
         separator(),
         text(" [ENTER] Send | [B] Back | [TAB] Switch field ") | dim |
             hcenter});

    return to_center(
        box | borderHeavy |
        color(addr_valid && amount_valid ? Color::CyanLight : Color::GrayDark) |
        size(WIDTH, EQUAL, 65) | size(HEIGHT, EQUAL, 20));
  });

  auto send_component = CatchEvent(send_form, [=, this](Event event) {
    if (event == Event::Character('b') || event == Event::Character('B') ||
        event == Event::Escape) {
      set_active_tab(WALLET_UI);
      return true;
    }

    if (event == Event::Return) {
      bool addr_valid = to_addr->size() == 42 && to_addr->substr(0, 2) == "0x";
      bool amount_valid = !amount_str->empty();

      if (!addr_valid) {
        *error_msg = "Invalid address";
        return true;
      }

      if (!amount_valid) {
        *error_msg = "Invalid amount";
        return true;
      }

      int idx = 0;

      WalletInfo info = actions->get_wallet();

      for (const auto &[id, asset] : *info.assets) {
        if (idx == *selected_asset) {
          *current_asset = asset;
          break;
        }

        idx++;
      }

      error_msg->clear();
      *selected_subtab = 1;
      return true;
    }

    return false;
  });

  auto preview_btn = Button("", [] {}, ButtonOption::Ascii());
  auto status_btn = Button("", [] {}, ButtonOption::Ascii());

  auto preview_form = Renderer(preview_btn, [=, this] {
    double gas_in_gwei = *final_gas;
    uint64_t gas_limit = current_asset->is_native ? 21000 : 65000;
    double fee_eth = (gas_limit * gas_in_gwei) / 1e9;

    return to_center(
        vbox(
            {text(" ⚠️  CONFIRM TRANSACTION ") | bold | color(Color::Yellow) |
                 hcenter,
             hbox({text(" To:     ") | dim, filler(),
                   text(*preview_addr) | color(Color::Cyan)}),

             hbox({text(" Amount: ") | dim, filler(),
                   text(amount_str->empty()
                            ? "--"
                            : *amount_str + " " + current_asset->symbol) |
                       color(Color::White)}),

             hbox({text(" Gas:    ") | dim, filler(),
                   text(fmt::format("{:.4f} Gwei", *final_gas)) |
                       color(Color::Yellow)}),

             hbox({text(" Fee:    ") | dim, filler(),
                   text(fmt::format("~{:.6f} ETH", fee_eth)) |
                       color(Color::GrayLight)}),

             hbox({text(" Network: ") | dim, filler(),
                   text(actions->get_current_network()) | color(Color::Green)}),

             error_msg->empty()
                 ? text("")
                 : text(" ✗ " + *error_msg) | color(Color::Red1) | hcenter,

             success_msg->empty()
                 ? text("")
                 : text(" ✓ " + *success_msg) | color(Color::GrayLight) |
                       hcenter | blink,
             separator(),
             text(" [ENTER] Confirm | [ESC] Back ") | dim | hcenter}) |
        borderHeavy | color(Color::YellowLight) | size(WIDTH, EQUAL, 60) |
        size(HEIGHT, EQUAL, 16)

    );
  });

  Component preview_component =
      CatchEvent(preview_form, [=, this](Event event) {
        if (event == Event::Return) {

          bool ok =
              actions->send_transaction(*to_addr, *current_asset, *amount_str,
                                        *final_gas, *custom_gas_limit);

          if (ok) {
            *success_msg = "Transaction sent!";
            to_addr->clear();
            amount_str->clear();
            *selected_subtab = 2;
          } else {
            *error_msg = "Failed to send transaction";
          }
          return true;
        } else if (event == Event::Escape) {
          *selected_subtab = 0;
          return true;
        }

        return false;
      });

  Component status_form = Renderer(status_btn, [=, this]() {
    auto [status, confirmed] = actions->get_current_tx_status();
    Element status_element{};
    switch (status) {
    case TxStatus::PENDING:
      *spin_idx = (*spin_idx + 1) % spinner->size();
      status_element =
          hbox({text(" " + spinner.get()->at(*spin_idx) + " ") |
                    color(Color::Yellow),
                text(" Waiting for confirmation... ") | color(Color::Yellow)}) |
          hcenter;
      break;

    case TxStatus::SUCCESS:
      status_element =
          vbox({text(" ✓ Confirmed! ") | color(Color::Green) | bold | hcenter,
                text(" Transaction included in block ") | dim | hcenter});
      break;

    case TxStatus::FAILED:
      status_element = vbox({
          text(" ✗ Transaction reverted ") | color(Color::Red1) | hcenter,
      });
      break;
    case TxStatus::ERROR:
      status_element =
          vbox({text(" Server error ") | color(Color::Red3) | hcenter});
      break;
    }

    return to_center(vbox(
        {text(" 📡 TRANSACTION STATUS ") | bold | color(Color::Cyan) | hcenter,
         separatorDouble() | color(Color::Cyan), filler(), status_element,
         filler(), separator(),
         text(" [ESC] Back to wallet ") | dim | hcenter}));
  });

  Component status_component = CatchEvent(status_form, [=, this](Event event) {
    if (event == Event::Escape) {
      *selected_subtab = 0;
      *tx_status = TxStatus::ERROR;
      set_active_tab(WALLET_UI);
      return true;
    }

    return false;
  });

  auto root =
      Container::Tab({send_component, preview_component, status_component},
                     selected_subtab.get());

  return Renderer(root, [=, this] {
    root->ChildAt(*selected_subtab)->TakeFocus();
    if (*selected_subtab == 0) {
      return send_component->Render();
    } else if (*selected_subtab == 1) {
      return preview_component->Render();
    }

    return status_component->Render();
  });
}
