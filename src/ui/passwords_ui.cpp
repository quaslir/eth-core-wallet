#include "ui/cli.hpp"
#include "ui/ftxui-components/input_component.hpp"
#include <ftxui/dom/elements.hpp>
Component CLI::render_password_setup(void) {

  auto pass_str = std::make_shared<secure_string>();
  auto field = input_(*pass_str, true);
  field->TakeFocus();
  auto first_stage = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return && !pass_str->empty()) {
      actions->set_password_for_wallet(std::move(*pass_str));

      set_active_tab(CONFIRM_PASSWORD);
      return true;
    } else if(event == Event::Escape) {
      set_active_tab(MAIN_MENU);
      return true;
    }

    return false;
  });

  return Renderer(first_stage, [=, this] {

    auto header = vbox({
      text(" 🛡️  CREATE MASTER PASSWORD ") | bold | hcenter | color(Color::Cyan),
      text(" This password protects your private keys ") | dim | hcenter
    });

    auto warning_box = vbox({
      text(" ⚠️  CRITICAL WARNING ") | bold | hcenter | color(Color::Yellow2),
      text(""),
      text(" This password will encrypt your wallet on disk. ") | hcenter,
      text(" If you forget it, your funds are LOST FOREVER. ") | hcenter | bold,
      text(" There is no 'Reset Password' in crypto. ") | hcenter | dim
    }) | border | color(Color::Red);

    auto input_box = hbox({
      text(" >>> ") | bold | color(Color::Yellow),
      field->Render() | flex
    }) | border | color(Color::GrayDark);


    auto content = vbox({
      header,
      separatorDouble(),
      filler(),
      warning_box,
      filler(),
      text(" INPUT IS HIDDEN FOR YOUR SECURITY ") | hcenter | dim,
      input_box
    });

    return to_center(content | borderHeavy | size(WIDTH, EQUAL, 70) | size(HEIGHT, EQUAL, 22));
  });
}

Component CLI::render_confirm_password_setup(void) {
  auto input_option = InputOption();
  input_option.multiline = false;
  input_option.password = true;

  auto second_pass = std::make_shared<secure_string>();
  auto field = input_(*second_pass, true);

  auto is_incorrect = std::make_shared<bool>(0);
  auto second_stage = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return) {
      auto first_pass = actions->get_password_for_wallet();

      if (first_pass == *second_pass) {

        actions->save_wallet();
        actions->update_balance();
        set_active_tab(WALLET_UI);
      } else {
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
  auto attempts = std::make_shared<std::size_t>(0);
  constexpr static size_t max_attempts = 3;
  auto user_input = std::make_shared<secure_string>();

  auto field = input_(*user_input, true);

  auto component = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return && !user_input->empty()) {

      if (actions->check_password(*user_input)) {

        actions->load_wallet();
        actions->update_balance();
        set_active_tab(WALLET_UI);
      } else {
        *attempts += 1;
        user_input->clear();
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
      status_info.push_back(text("[!] ACCESS DENIED: INVALID PASSWORD") | bold |
                            color(Color::Red) | hcenter);

    if (remaining == 1) {
      status_info.push_back(text("!!! FINAL ATTEMPT: WIPE ON FAILURE !!!") |
                            bold | color(Color::RedLight) | blink | hcenter);
    } else  {
      status_info.push_back(text("Remaining attempts: " + std::to_string(remaining)) |
                            dim | hcenter);
    }
  }

  auto header = vbox({
    text(" 🔒 UNLOCK WALLET ") | bold | hcenter | color(Color::Cyan),
    separatorDouble(),
    text(" Encryption active. Enter master password to access private keys. ") | dim | hcenter
  });

  auto status_box = vbox({
    text(""),
    vbox(std::move(status_info)),

  });

  auto input_box = vbox({
    text(" PASSWORD >>> ") | bold | color(Color::Cyan),
    field->Render() | flex | border | color(Color::GrayDark) | focus
  }) | size(HEIGHT, EQUAL, 6);


    auto content = vbox({
      header,
      filler(),
      status_box,
      filler(),
      input_box,
      filler(),
    });

    return to_center(content | borderHeavy | size(WIDTH, EQUAL, 60) | size(HEIGHT, EQUAL, 14));
  });
}
