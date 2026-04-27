#pragma once

#include <curl/curl.h>
#include <stdint.h>
#include <string_view>
#include <vector>

namespace http {
std::string post_request(const std::string &url, const std::string &data);
std::string get_request(const std::string &url);
size_t http_callback(void *contents, size_t size, size_t number,
                     std::string *result);
} // namespace http