#pragma once
#include "config/configuration.hpp"
#include "core/secure_bytes_data.hpp"
#include "utils/tech_utils.hpp"
#include <cstddef>

#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>
#include <memory>

using namespace ftxui;

class ParagraphNode : public Node {
private:
  secure_string data_;

public:
  explicit ParagraphNode(secure_string data) : data_(std::move(data)) {}

  void ComputeRequirement(void) override {
    requirement_.min_x = data_.size();
    requirement_.min_y = MIN_Y;

    requirement_.flex_grow_x = 1;
    requirement_.flex_grow_y = 1;
  }

  void Render(Screen &screen) override {
    int width = box_.x_max - box_.x_min + 1;
    if (width <= 0)
      return;

    int start_x = box_.x_min;
    int start_y = box_.y_min;

    for (size_t i = 0; i < data_.size(); i++) {
      int relative_x = i % width;
      int relative_y = i / width;

      int target_x = start_x + relative_x;
      int target_y = start_y + relative_y;

      if (target_y > box_.y_max)
        break;

      screen.at(target_x, target_y) = static_cast<char>(data_[i]);
    }
  }
};

inline Element paragraph_(const secure_string &data) {
  return std::make_shared<ParagraphNode>(data);
}
