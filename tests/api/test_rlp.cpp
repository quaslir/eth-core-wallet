#include <catch2/catch_test_macros.hpp>
#include "api/rlp.hpp"

static bytes_data B(std::initializer_list<uint8_t> l) { return bytes_data(l); }

TEST_CASE("encode_length", "[rlp]") {
    SECTION("1")   { CHECK(rlp::encode_length(1)      == B({0x01})); }
    SECTION("127") { CHECK(rlp::encode_length(127)     == B({0x7f})); }
    SECTION("128") { CHECK(rlp::encode_length(128)     == B({0x80})); }
    SECTION("255") { CHECK(rlp::encode_length(255)     == B({0xff})); }
    SECTION("256") { CHECK(rlp::encode_length(256)     == B({0x01, 0x00})); }
    SECTION("257") { CHECK(rlp::encode_length(257)     == B({0x01, 0x01})); }
    SECTION("1000"){ CHECK(rlp::encode_length(1000)    == B({0x03, 0xe8})); }
    SECTION("65535"){ CHECK(rlp::encode_length(65535)  == B({0xff, 0xff})); }
    SECTION("65536"){ CHECK(rlp::encode_length(65536)  == B({0x01, 0x00, 0x00})); }
    SECTION("100000"){ CHECK(rlp::encode_length(100000)== B({0x01, 0x86, 0xa0})); }
}

TEST_CASE("encode_uint", "[rlp]") {
    SECTION("0")   { CHECK(rlp::encode_uint(0)   == B({0x80})); }

    SECTION("1")   { CHECK(rlp::encode_uint(1)   == B({0x01})); }
    SECTION("9")   { CHECK(rlp::encode_uint(9)   == B({0x09})); }
    SECTION("15")  { CHECK(rlp::encode_uint(15)  == B({0x0f})); }
    SECTION("127") { CHECK(rlp::encode_uint(127) == B({0x7f})); }

    SECTION("128") { CHECK(rlp::encode_uint(128) == B({0x81, 0x80})); }
    SECTION("200") { CHECK(rlp::encode_uint(200) == B({0x81, 0xc8})); }
    SECTION("255") { CHECK(rlp::encode_uint(255) == B({0x81, 0xff})); }

    SECTION("256") { CHECK(rlp::encode_uint(256)   == B({0x82, 0x01, 0x00})); }
    SECTION("1000"){ CHECK(rlp::encode_uint(1000)  == B({0x82, 0x03, 0xe8})); }
    SECTION("65535"){ CHECK(rlp::encode_uint(65535)== B({0x82, 0xff, 0xff})); }

    SECTION("65536")  { CHECK(rlp::encode_uint(65536)  == B({0x83, 0x01, 0x00, 0x00})); }
    SECTION("1000000"){ CHECK(rlp::encode_uint(1000000)== B({0x83, 0x0f, 0x42, 0x40})); }

    SECTION("21000")  { CHECK(rlp::encode_uint(21000)  == B({0x82, 0x52, 0x08})); }
    SECTION("20e9 gwei") {
        CHECK(rlp::encode_uint(20000000000ULL) == B({0x85, 0x04, 0xa8, 0x17, 0xc8, 0x00}));
    }
}

TEST_CASE("encode_bytes", "[rlp]") {

    SECTION("empty") { CHECK(rlp::encode_bytes({}) == B({0x80})); }

    SECTION("single 0x00") { CHECK(rlp::encode_bytes({0x00}) == B({0x81, 0x00})); }
    SECTION("single 0x7f") { CHECK(rlp::encode_bytes({0x7f}) == B({0x81, 0x7f})); }
    SECTION("single 0x80") { CHECK(rlp::encode_bytes({0x80}) == B({0x81, 0x80})); }


    SECTION("2 bytes") {
        CHECK(rlp::encode_bytes({0x01, 0x02}) == B({0x82, 0x01, 0x02}));
    }
    SECTION("3 bytes dog") {

        CHECK(rlp::encode_bytes({0x64, 0x6f, 0x67}) == B({0x83, 0x64, 0x6f, 0x67}));
    }
    SECTION("20 bytes address") {
        bytes_data addr(20, 0xab);
        bytes_data expected = {0x94};
        expected.insert(expected.end(), addr.begin(), addr.end());
        CHECK(rlp::encode_bytes(addr) == expected);
    }
    SECTION("55 bytes exact") {
        bytes_data data(55, 0x01);
        bytes_data expected = {static_cast<uint8_t>(0x80 + 55)};
        expected.insert(expected.end(), data.begin(), data.end());
        CHECK(rlp::encode_bytes(data) == expected);
    }


    SECTION("56 bytes") {
        bytes_data data(56, 0x01);

        bytes_data expected = {0xb8, 0x38};
        expected.insert(expected.end(), data.begin(), data.end());
        CHECK(rlp::encode_bytes(data) == expected);
    }
    SECTION("100 bytes") {
        bytes_data data(100, 0xff);
        bytes_data expected = {0xb8, 0x64};
        expected.insert(expected.end(), data.begin(), data.end());
        CHECK(rlp::encode_bytes(data) == expected);
    }
    SECTION("1000 bytes") {
        bytes_data data(1000, 0xaa);
        bytes_data expected = {0xb9, 0x03, 0xe8};
        expected.insert(expected.end(), data.begin(), data.end());
        CHECK(rlp::encode_bytes(data) == expected);
    }
}

TEST_CASE("encode_list", "[rlp]") {

    SECTION("empty list") {
        CHECK(rlp::encode_list({}) == B({0xc0}));
    }

    SECTION("list [1]") {

        CHECK(rlp::encode_list({rlp::encode_uint(1)}) == B({0xc1, 0x01}));
    }


    SECTION("list [dog, cat]") {
        bytes_data dog = rlp::encode_bytes({0x64, 0x6f, 0x67});
        bytes_data cat = rlp::encode_bytes({0x63, 0x61, 0x74});

        bytes_data expected = {0xc8, 0x83, 0x64, 0x6f, 0x67, 0x83, 0x63, 0x61, 0x74};
        CHECK(rlp::encode_list({dog, cat}) == expected);
    }


    SECTION("list [1, 2]") {
        CHECK(rlp::encode_list({
            rlp::encode_uint(1),
            rlp::encode_uint(2)
        }) == B({0xc2, 0x01, 0x02}));
    }


    SECTION("list total == 55") {
        bytes_data item(54, 0x01);
        bytes_data enc = rlp::encode_bytes(item);
        size_t total = enc.size();
        uint8_t prefix = static_cast<uint8_t>(0xc0 + total);
        bytes_data expected = {prefix};
        expected.insert(expected.end(), enc.begin(), enc.end());
        CHECK(rlp::encode_list({enc}) == expected);
    }


    SECTION("list total > 55") {
        bytes_data item(60, 0x01);
        bytes_data enc = rlp::encode_bytes(item);
        bytes_data expected = {0xf8, static_cast<uint8_t>(enc.size())};
        expected.insert(expected.end(), enc.begin(), enc.end());
        CHECK(rlp::encode_list({enc}) == expected);
    }


    SECTION("eth tx structure") {
        auto encoded = rlp::encode_list({
            rlp::encode_uint(0),
            rlp::encode_uint(21000000000ULL),
            rlp::encode_uint(21000),
            rlp::encode_bytes(bytes_data(20, 0xab)),
            rlp::encode_uint(1000000000000000000ULL),
            rlp::encode_bytes({})
        });

        CHECK(encoded[0] == 0xe9);
        CHECK(encoded.size() == 42);
    }
}
