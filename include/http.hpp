#pragma once

#include <curl/curl.h>
#include <vector>
#include <stdint.h>
#include <string_view>
using bytes_data = std::vector<uint8_t>;

namespace http {
    bytes_data post_request(const std::string&url, const bytes_data& data);
    bytes_data get_request(const std::string& url);
    size_t http_callback(void * contents, size_t size, size_t number, bytes_data* result);
}