#pragma once
#include "core/secure_bytes_data.hpp"
#include "text_component.hpp"
#include "utils/tech_utils.hpp"
#include <cstdint>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>
#include <functional>
#include <memory>

using namespace ftxui;

class SecureInput : public ComponentBase {
private:
  secure_string &data_;
  bool password_ = false;
  std::function<void()> on_change_;

public:
  explicit SecureInput(secure_string &data, bool password, std::function<void()> on_change)
      : data_(data), password_(password), on_change_(on_change){}

  bool Focusable(void) const override { return true; }

  bool OnEvent(Event event) override {

    bool changed = false;

    if (event.is_character()) {
      data_.push_back(static_cast<uint8_t>(event.character()[0]));
      changed = true;
    }

    if (event == Event::Backspace && !data_.empty()) {
      data_.pop_back();
      changed = true;
    }

    if(changed && on_change_) {
      on_change_();
    }

    return changed;
  }

  Element Render(void) override {
    Element content;

    auto cursor = text("|") | blink;



    if (password_) {
      secure_string hidden_input(data_.size(), static_cast<uint8_t>('*'));
      content = text_(hidden_input);
    } else {
      content = text_(data_);
    }

    Element content_el = Focused() ? (
      hbox({
        content,
        cursor
      })
    ) : (
      content
    );


    auto decorator = Focused() ? (borderHeavy | color(Color::Cyan)) : border | color(Color::GrayDark);

    return content_el | decorator | size(WIDTH, EQUAL, 50);
  }
};

inline Component input_(secure_string &data, bool password = false, std::function<void()> on_change = nullptr) {
  return std::make_shared<SecureInput>(data, password, on_change);
}
