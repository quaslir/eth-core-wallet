#include "api/rlp.hpp"
#include "core/secure_bytes_data.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>


namespace rlp {
    bytes_data encode_bytes(const bytes_data& data) {
        if(data.size() <=55) {
            uint8_t prefix = 0x80 + data.size();
            bytes_data encoded = {prefix};
            encoded.insert(encoded.end(),data.begin(), data.end());
            return encoded;
        }

        bytes_data len_encoded = encode_length(data.size());
        bytes_data encoded = {static_cast<uint8_t>(0xb7 + len_encoded.size())};
        encoded.insert(encoded.end(), len_encoded.begin(), len_encoded.end());
        encoded.insert(encoded.end(), data.begin(), data.end());
        return encoded;
    }
    bytes_data encode_uint(uint64_t value) {
        if(value == 0x00) return {0x80};
        if(value <= 0x7f) {
             return {static_cast<uint8_t>(value)};
        }
        bytes_data encoded;

        while(value > 0) {
            encoded.insert(encoded.begin(), value & 0xff);
            value = value >> 8;
        }

        return encode_bytes(encoded);

    }
    bytes_data encode_list(const std::vector<bytes_data>& items) {

        auto add_items = [&items](bytes_data& data) -> void {
            for(const auto & item : items) {
            data.insert(data.end(), item.begin(), item.end());
            }
        };


        size_t total = 0;
        for(const auto& item: items) total += item.size();
        if(total <= 55) {
            uint8_t prefix = 0xc0 + total;
            bytes_data encoded = {prefix};
            add_items(encoded);
            return encoded;
        }

        bytes_data len_encoded = encode_length(total);
        bytes_data encoded = {static_cast<uint8_t>(0xf7 + len_encoded.size())};
        encoded.insert(encoded.end(), len_encoded.begin(), len_encoded.end());
        add_items(encoded);
        return encoded;

    }

    bytes_data encode_length(size_t length) {
        bytes_data encoded;

        while(length > 0) {
            encoded.insert(encoded.begin(), length & 0xff);
            length = length >> 8;
        }

        return encoded;
    }
}
