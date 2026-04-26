#pragma once

#include <cstdint>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>
#include <memory>
#include <vector>

using namespace ftxui;
using bytes_data = std::vector<uint8_t>;

class BytesTextNode : public Node {
private:
  bytes_data data_;

public:
  void ComputeRequirement(void) override {
    requirement_.min_x = 2 + data_.size() * 2;
    requirement_.min_y = 1;
  }

  void Render(Screen &screen) override {
    int x = box_.x_min;
    int y = box_.y_min;

    if (x > box_.x_max || y > box_.y_max)
      return;

    static const char *hex_digits = "0123456789abcdef";

    for (const auto &b : data_) {
      if (x + 1 > box_.x_max)
        break;

      screen.at(x++, y) = hex_digits[(b >> 4) & 0x0F];
      screen.at(x++, y) = hex_digits[b & 0x0F];
    }
  }

  explicit BytesTextNode(bytes_data data) : data_(std::move(data)) {}
};

inline Element byte_text(const bytes_data &data) {
  return std::make_shared<BytesTextNode>(data);
}
