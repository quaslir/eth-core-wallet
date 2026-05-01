#pragma once
#include "core/secure_bytes_data.hpp"
#include "text_component.hpp"
#include "utils/tech_utils.hpp"
#include <cstdint>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>
#include <memory>

using namespace ftxui;

class SecureInput : public ComponentBase {
private:
  secure_string &data_;
  bool password_ = false;

public:
  explicit SecureInput(secure_string &data, bool password)
      : data_(data), password_(password) {}

  bool Focusable(void) const override { return true; }

  bool OnEvent(Event event) override {
    if (event.is_character()) {
      data_.push_back(static_cast<uint8_t>(event.character()[0]));
      return true;
    }

    if (event == Event::Backspace && !data_.empty()) {
      data_.pop_back();
      return true;
    }

    return false;
  }

  Element Render(void) override {
    Element content;
    if (password_) {
      secure_string hidden_input(data_.size(), static_cast<uint8_t>('*'));
      content = text_(hidden_input);
    } else {
      content = text_(data_);
    }

    auto decorator = Focused() ? (borderHeavy | color(Color::Yellow)) : border;

    return content | decorator | size(WIDTH, EQUAL, 30);
  }
};

inline Component input_(secure_string &data, bool password = false) {
  return std::make_shared<SecureInput>(data, password);
}
