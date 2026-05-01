#include <catch2/catch_all.hpp>
#include <string>
#include "catch2/catch_test_macros.hpp"
#include "core/secure_bytes_data.hpp"
#include "utils/utils.hpp"
#include "core/mnemonic.hpp"

TEST_CASE("Mnemonic generation basic flow", "[mnemonic][crypto]") {
    MnemonicGenerator mnem_gen;
    SECTION("128-bit entropy (12 words)") {
        Config config{};
        config.bit_length = 128;
        secure_string seed_phrase = mnem_gen.generateMnemonic(config);
        REQUIRE(!seed_phrase.empty());
        REQUIRE(mnem_gen.mnemonic_is_correct(seed_phrase));
    }

    SECTION("256-bit entropy (24 words)"){
        Config config{};
        config.bit_length = 256 ;
        secure_string seed_phrase = mnem_gen.generateMnemonic(config);
        REQUIRE(!seed_phrase.empty());
        REQUIRE(mnem_gen.mnemonic_is_correct(seed_phrase));
    }
}



TEST_CASE("Seed phrase generation test", "[mnemonic][entropy]") {
auto test_data = core_test_utils::get_vector_tests();

REQUIRE_FALSE(test_data.empty());

MnemonicGenerator mnemgen;

for(auto& entry : test_data) {
    SECTION("Vector " + std::string{entry.seed_phrase.substr(0, 20)} + "...") {
        secure_string seed_phrase = mnemgen.__generateMnemonic(entry.entropy);
        REQUIRE(seed_phrase  == entry.seed_phrase);
        auto res = mnemgen.generateSeed(seed_phrase);
        REQUIRE(res == entry.seed);
    }
}
}



TEST_CASE("Mnemonic massive stress test", "[mnemonic][security][stress]") {
    MnemonicGenerator mnem_gen;

    auto invalid_phrase = GENERATE(
        "abandon`abandon`abandon`abandon`abandon`abandon`abandon`abandon`abandon`abandon`abandon`about",
        "a b c d e f g h i j k l m n o p q r s t u v w x y z",
        "abandon\0"
    );

    REQUIRE_FALSE(mnem_gen.mnemonic_is_correct(invalid_phrase));
}
