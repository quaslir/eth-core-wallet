#include "ui/cli.hpp"
#include "ui/ftxui-components/input_component.hpp"
Component CLI::render_password_setup(void) {

  auto input_option = InputOption();
  input_option.multiline = false;
  input_option.password = true;
  auto pass_str = std::make_shared<secure_string>();
  auto field = input_(*pass_str, true);
  field->TakeFocus();
  auto first_stage = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return && !pass_str->empty()) {
      actions->set_password_for_wallet(std::move(*pass_str));

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
  auto input_option = InputOption();
  auto attempts = std::make_shared<std::size_t>(0);
  constexpr static size_t max_attempts = 3;
  auto user_input = std::make_shared<secure_string>();
  input_option.multiline = false;
  input_option.password = true;
  auto field = input_(*user_input, true);

  auto component = CatchEvent(field, [=, this](Event event) {
    if (event == Event::Return && !user_input->empty()) {

      if (actions->check_password(*user_input)) {

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
