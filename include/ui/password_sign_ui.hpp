#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>
#include "core/secure_bytes_data.hpp"
using namespace ftxui;


class PasswordUI {
    public :
        static Component create(const std::string & title, std::function<bool(const secure_string&)> verify,
            std::function<void()> on_success, std::function<void()> on_cancel);

};
