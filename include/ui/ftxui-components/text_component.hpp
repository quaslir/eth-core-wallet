#pragma once

#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>
#include <memory>

#include "core/secure_bytes_data.hpp"
using namespace ftxui;

class TextNode : public Node {
private:
  secure_string data_;

public:
  explicit TextNode(secure_string data) : data_(std::move(data)) {}

  void ComputeRequirement(void) override {
    requirement_.min_x = static_cast<int>(data_.size());
    requirement_.min_y = 1;
  }

  void Render(Screen &screen) override {
    int x = box_.x_min;
    int y = box_.y_min;

    if (x > box_.x_max || y > box_.y_max)
      return;

    for (const auto &byte : data_) {
      if (x > box_.x_max)
        break;
      screen.at(x++, y) = static_cast<char>(byte);
    }
  }
};

inline Element text_(const secure_string &data) {
  return std::make_shared<TextNode>(data);
}
