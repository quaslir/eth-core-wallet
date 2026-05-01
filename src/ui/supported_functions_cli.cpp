#include "ui/cli.hpp"
#include <ftxui/dom/elements.hpp>

void CLI::set_actions(IWalletActions *act) { actions = act; }

void CLI::set_active_tab(int tab) {
  active_tab = tab;
  screen.Post(Event::Custom);
}

Element CLI::to_center(Element box) {
  return vbox({
      filler(),
      hbox({filler(), box, filler()}),
      filler(),
  });
}

int CLI::get_tab_data(void) const { return active_tab; }

ButtonOption CLI::create_button(const std::string &label, Color c) const {
  auto opt = ButtonOption::Ascii();

  opt.transform = [label, c](const EntryState &state) {
    auto t = text(" " + label + " ");
    
    if(state.focused) {
      return t | bold | inverted | color(c);
    }

    return t | color(Color::DarkSlateGray1) | dim;
  };

  return opt;
}