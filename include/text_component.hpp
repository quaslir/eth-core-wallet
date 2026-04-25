#pragma once
#include "tech_utils.hpp"
#include <cstdint>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>
#include <memory>
#include <vector>

using namespace ftxui;
using bytes_data = std::vector<uint8_t>;

class TextNode : public Node {
    private:
     bytes_data data_;

    public:
    explicit TextNode(bytes_data data) : data_(std::move(data)) {}
    ~TextNode() {
        if(!data_.empty()) {
            tech_utils::clear(data_);
        }
    }
    void ComputeRequirement(void) override {
        requirement_.min_x = 2 + data_.size() * 2;
        requirement_.min_y = 1;
    }

    void Render(Screen & screen) override {
        int x = box_.x_min;
        int y = box_.y_min;

        if(x > box_.x_max || y > box_.y_max) return;

        for(const auto& byte : data_) {
            if(x > box_.x_max) break;
             screen.at(x++, y) = static_cast<char>(byte);
        }
    }

};


inline Element text_(const bytes_data& data) {
    return std::make_shared<TextNode>(data);
}
