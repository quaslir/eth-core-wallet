#include "ui/password_sign_ui.hpp"
#include "core/secure_bytes_data.hpp"
#include "ui/ftxui-components/input_component.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>

Component PasswordUI::create(const std::string & title, std::function<bool(const secure_string&)> verify,
    std::function<void()> on_success, std::function<void()> on_cancel) {
        auto password = std::make_shared<secure_string>();
        auto error_msg = std::make_shared<secure_string>();

        auto input = input_(*password, true, [=]{error_msg->clear();});
        input->TakeFocus();

       auto form =  Renderer(input, [=] {
                return vbox({
                    vbox({
                    text(" 🔐 " + title + " ") | bold | color(Color::Cyan) | hcenter,
                    separatorDouble() | color(Color::Cyan),
                    text(" Enter password to confirm ") | dim | hcenter,
                    separator(),
                    hbox({
                        text(" >>> ") | color(Color::Yellow),
                        input->Render() | flex,
                    }),

                    error_msg->empty() ? emptyElement() : text_(*error_msg) | hcenter | bold | color(Color::Red),

                    separator(),
                    text(" [ENTER] Confirm | [ESC] Cancel ") | dim | hcenter
                }) | borderHeavy | color(Color::Yellow) | size(WIDTH, EQUAL, 50) | size(HEIGHT, EQUAL, 12)

                }) | center;
        });

       return CatchEvent(form, [=] (Event event) {
           if(event == Event::Return) {
               if(verify(*password)) {
                   error_msg->clear();
                   password->clear();
                   on_success();
               } else {
                   *error_msg = "Invalid password";
                   password->clear();
               }

               return true;
           } else if(event == Event::Escape) {
               error_msg->clear();
               password->clear();
               on_cancel();
               return true;
           }
           return false;
       });

    }
