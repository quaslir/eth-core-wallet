#include "eip712.hpp"
#include "Keccak256.hpp"

#include <algorithm>
#include <set>
#include <stdexcept>

namespace eip712 {
namespace detail {

static bytes_t hex_string_to_bytes(std::string hex) {
    if (hex.size() >= 2 && hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X'))
        hex = hex.substr(2);
    if (hex.size() % 2 != 0)
        hex = "0" + hex;

    bytes_t raw(hex.size() / 2);
    for (size_t i = 0; i < raw.size(); i++)
        raw[i] = static_cast<uint8_t>(std::stoul(hex.substr(i * 2, 2), nullptr, 16));
    return raw;
}

static bytes_t hex_to_bytes32_right(std::string hex) {
    bytes_t raw = hex_string_to_bytes(hex);
    bytes_t out(32, 0);
    size_t n = std::min<size_t>(raw.size(), 32);
    std::copy(raw.end() - n, raw.end(), out.begin() + (32 - n));
    return out;
}

static bytes_t hex_to_bytes32_left(std::string hex) {
    bytes_t raw = hex_string_to_bytes(hex);
    bytes_t out(32, 0);
    size_t n = std::min<size_t>(raw.size(), 32);
    std::copy(raw.begin(), raw.begin() + n, out.begin());
    return out;
}

static bytes_t decimal_to_bytes32(const std::string& num_str) {
    std::vector<int> digits;
    for (char c : num_str)
        digits.push_back(c - '0');

    bytes_t out;
    while (true) {
        int remainder = 0;
        std::vector<int> quotient;
        bool all_zero = true;

        for (int d : digits) {
            int cur = remainder * 10 + d;
            int q = cur / 256;
            remainder = cur % 256;
            quotient.push_back(q);
            if (q != 0)
                all_zero = false;
        }

        out.push_back(static_cast<uint8_t>(remainder));
        if (all_zero)
            break;
        digits = quotient;
    }

    std::reverse(out.begin(), out.end());
    bytes_t padded(32, 0);
    size_t n = std::min<size_t>(out.size(), 32);
    std::copy(out.end() - n, out.end(), padded.begin() + (32 - n));
    return padded;
}

static void find_deps(const std::string& type, const nlohmann::json& types, std::set<std::string>& deps) {
    std::string base = type;
    size_t bracket = base.find('[');
    if (bracket != std::string::npos)
        base = base.substr(0, bracket);

    if (!types.contains(base) || deps.count(base))
        return;

    deps.insert(base);
    for (const auto& field : types.at(base))
        find_deps(field.at("type").get<std::string>(), types, deps);
}

} // namespace detail

bytes_t keccak256(const bytes_t& input) {
    bytes_t out(Keccak256::HASH_LEN);
    Keccak256::getHash(input.data(), input.size(), out.data());
    return out;
}

std::string encode_type(const std::string& primary_type, const nlohmann::json& types) {
    std::set<std::string> deps;
    detail::find_deps(primary_type, types, deps);
    deps.erase(primary_type);

    auto encode_one = [&](const std::string& name) {
        std::string out = name + "(";
        const auto& fields = types.at(name);
        for (size_t i = 0; i < fields.size(); i++) {
            if (i)
                out += ",";
            out += fields[i].at("type").get<std::string>() + " " + fields[i].at("name").get<std::string>();
        }
        out += ")";
        return out;
    };

    std::string result = encode_one(primary_type);
    for (const auto& dep : deps)
        result += encode_one(dep);
    return result;
}

bytes_t type_hash(const std::string& primary_type, const nlohmann::json& types) {
    std::string encoded = encode_type(primary_type, types);
    bytes_t bytes(encoded.begin(), encoded.end());
    return keccak256(bytes);
}

bytes_t encode_value(const std::string& type, const nlohmann::json& value, const nlohmann::json& types) {
    size_t bracket = type.rfind('[');
    if (bracket != std::string::npos) {
        std::string elem_type = type.substr(0, bracket);
        bytes_t concat;
        for (const auto& item : value) {
            bytes_t enc = encode_value(elem_type, item, types);
            concat.insert(concat.end(), enc.begin(), enc.end());
        }
        return keccak256(concat);
    }

    if (types.contains(type))
        return hash_struct(type, value, types);

    if (type == "string") {
        std::string s = value.get<std::string>();
        bytes_t raw(s.begin(), s.end());
        return keccak256(raw);
    }

    if (type == "bytes") {
        bytes_t raw = detail::hex_string_to_bytes(value.get<std::string>());
        return keccak256(raw);
    }

    if (type == "bool") {
        bool b = value.is_boolean() ? value.get<bool>() : (value.get<std::string>() == "true");
        bytes_t out(32, 0);
        out[31] = b ? 1 : 0;
        return out;
    }

    if (type == "address")
        return detail::hex_to_bytes32_right(value.get<std::string>());

    if (type.rfind("uint", 0) == 0 || type.rfind("int", 0) == 0) {
        std::string s = value.is_number_integer()
            ? std::to_string(value.get<long long>())
            : value.get<std::string>();

        bool is_hex = s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X');
        return is_hex ? detail::hex_to_bytes32_right(s) : detail::decimal_to_bytes32(s);
    }

    if (type.rfind("bytes", 0) == 0)
        return detail::hex_to_bytes32_left(value.get<std::string>());

    throw std::runtime_error("eip712: unsupported type '" + type + "'");
}

bytes_t hash_struct(const std::string& primary_type, const nlohmann::json& data, const nlohmann::json& types) {
    bytes_t out = type_hash(primary_type, types);
    for (const auto& field : types.at(primary_type)) {
        std::string name = field.at("name").get<std::string>();
        std::string type = field.at("type").get<std::string>();
        bytes_t enc = encode_value(type, data.at(name), types);
        out.insert(out.end(), enc.begin(), enc.end());
    }
    return keccak256(out);
}

bytes_t hash_typed_data(const nlohmann::json& typed_data) {
    const nlohmann::json& types = typed_data.at("types");
    std::string primary_type = typed_data.at("primaryType").get<std::string>();

    bytes_t domain_sep = hash_struct("EIP712Domain", typed_data.at("domain"), types);
    bytes_t msg_hash = hash_struct(primary_type, typed_data.at("message"), types);

    bytes_t payload = {0x19, 0x01};
    payload.insert(payload.end(), domain_sep.begin(), domain_sep.end());
    payload.insert(payload.end(), msg_hash.begin(), msg_hash.end());

    return keccak256(payload);
}

} // namespace eip712
