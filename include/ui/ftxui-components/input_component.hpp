#pragma once
#include "tech_utils.hpp"
#include "text_component.hpp"
#include <cstdint>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>
#include <memory>
#include <vector>

using namespace ftxui;
using bytes_data = std::vector<uint8_t>;
class SecureInput : public ComponentBase {
private:
  bytes_data &data_;
  bool password_ = false;

public:
  explicit SecureInput(bytes_data &data, bool password)
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
      bytes_data hidden_input(data_.size(), static_cast<uint8_t>('*'));
      content = text_(hidden_input);
    } else {
      content = text_(data_);
    }

    auto decorator = Focused() ? (borderHeavy | color(Color::Yellow)) : border;

    return content | decorator | size(WIDTH, EQUAL, 30);
  }
};

inline Component input_(bytes_data &data, bool password = false) {
  return std::make_shared<SecureInput>(data, password);
}
