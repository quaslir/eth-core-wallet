#include "cli.hpp"
#include "config.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <openssl/crypto.h>
#include <string>
#include <string_view>

Component CLI::set_bit_length(void) {
static int selected = 0;
const static std::vector<std::string> entries = {
    "[1] 128 BITS (12 WORDS)",
    "[2] 256 BITS (24 WORDS)"
};

const static std::vector<std::string> descriptions = {
    "Standard security. Ideal for Hot wallets and frequent transactions. Easy to write and store.",
    "Military-grade security. Recommended for \"Cold\" storage. Impossible to brute-force, even in theory."
};
auto menu = Menu(&entries, &selected);

auto component =  CatchEvent(menu, [this] (Event event) {
if(event == Event::Character('b') || event == Event::Character('B') || event == Event::Escape) {
    set_active_tab(CONFIG_MENU);
    return true;
}
else if(event == Event::Return) {
    if(selected == 0) {
        actions->change_bit_length(128);
    }

    if(selected == 1) {
        actions->change_bit_length(256);
    }
    set_active_tab(CONFIG_MENU);
    return true;
}
return false;
});


return Renderer(component, [=, this]{
    auto box = vbox({

        text("SELECT ENTROPY LENGTH") | bold | hcenter | color(Color::Cyan2),
        separatorLight(),

        menu->Render(),
        separatorLight(),
        text("[B] BACK TO CONFIG")
    });

    return to_center(box);
});
}

Component CLI::handle_extra_entropy(void) {
    static int selected = 0;
    const static std::vector<std::string> entries = {
        "[1] SYSTEM ONLY (OS CSPRNG)",
        "[2] SYSTEM + USER MIX (SHA-256 Hybrid)"
    };

    const static std::vector<std::string> descriptions = {
        "Standard mode. Uses /dev/urandom or BCryptGenRandom.\nReliable and tested by millions of users.",
        "Paranoid mode. Mixes OS random with your secret \ninput using SHA-256. Protects against OS backdoors."
    };
    auto active_sub_tab = std::make_shared<int>(0);
    auto menu = Menu(&entries, &selected);

    auto component =  CatchEvent(menu, [=, this] (Event event) {
    if(event == Event::Character('b') || event == Event::Character('B') || event == Event::Escape) {
        set_active_tab(CONFIG_MENU);
        return true;
    }
    else if(event == Event::Return) {
        if(selected == 0) {
            actions->set_extra_entropy(std::string_view{});
            set_active_tab(CONFIG_MENU);
        }

        if(selected == 1) {
            *active_sub_tab = 1;
        }

        return true;
    }
    return false;
    });

    auto user_input = std::make_shared<std::string>("");

    auto input_option = InputOption();
    input_option.multiline = false;
    input_option.password = true;
    auto field = Input(user_input.get(), "Enter extra entropy...", input_option);
    field->TakeFocus();

    auto text_box = CatchEvent(field, [=, this] (Event event) {
        if(event == Event::Return) {
            actions->set_extra_entropy(*user_input);

            OPENSSL_cleanse(user_input->data(), user_input->size()); //fix
            user_input->clear();
            set_active_tab(CONFIG_MENU);
            *active_sub_tab = 0;
            return true;
        }

        return false;
    });
    auto container = Container::Tab({
        component,
        text_box
    }, active_sub_tab.get());
    return Renderer(container, [=, this] {
        Element box;
        if(*active_sub_tab == 1) {
            box = vbox({
            text("ENTER EXTRA ENTROPY") | bold | hcenter | color(Color::Cyan3),
            separatorLight(),
            text_box->Render()
            });
        } else {
        box = vbox({
            text("SELECT EXTRA ENTROPY SETTING") | bold | hcenter | color(Color::Cyan2),
            separatorLight(),

            menu->Render(),
            separatorLight(),
            text("[B] BACK TO CONFIG")
        });
        }
        return to_center(box);
    });

}
