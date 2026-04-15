#include "cli.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

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
