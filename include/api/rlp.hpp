
#include "core/secure_bytes_data.hpp"
#include <cstddef>
#include <cstdint>
namespace rlp {
bytes_data encode_bytes(const bytes_data &data);
bytes_data encode_uint(uint64_t value);
bytes_data encode_list(const std::vector<bytes_data> &items);
bytes_data encode_length(size_t length);
} // namespace rlp
