#pragma once
#include "configuration.hpp"
#include "tech_utils.hpp"
#include <cstddef>
#include <cstdint>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>
#include <memory>
#include <vector>
using namespace ftxui;
using bytes_data = std::vector<uint8_t>;

class ParagraphNode : public Node {
private:
  bytes_data data_;

public:
  explicit ParagraphNode(bytes_data data) : data_(std::move(data)) {}
  ~ParagraphNode() {
    if (!data_.empty()) {
      tech_utils::clear(data_);
    }
  }
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

inline Element paragraph_(const bytes_data &data) {
  return std::make_shared<ParagraphNode>(data);
}
