#include <catch2/catch_all.hpp>
#include "catch2/catch_message.hpp"
#include "catch2/catch_test_macros.hpp"
#include "utils/tech_utils.hpp"
#include "utils/utils.hpp"
#include "core/mnemonic.hpp"

TEST_CASE("Mnemonic generation basic flow", "[mnemonic][crypto]") {
    MnemonicGenerator mnem_gen;
    SECTION("128-bit entropy (12 words)") {
        Config config{};
        config.bit_length = 128;
        bytes_data seed_phrase = mnem_gen.generateMnemonic(config);
        REQUIRE(!seed_phrase.empty());
        REQUIRE(mnem_gen.mnemonic_is_correct(std::string_view{reinterpret_cast<const char *>(seed_phrase.data()), seed_phrase.size()}));
    }

    SECTION("256-bit entropy (24 words)"){
        Config config{};
        config.bit_length = 256 ;
        bytes_data seed_phrase = mnem_gen.generateMnemonic(config);
        REQUIRE(!seed_phrase.empty());
        REQUIRE(mnem_gen.mnemonic_is_correct(std::string_view{reinterpret_cast<const char *>(seed_phrase.data()), seed_phrase.size()}));
    }
}



TEST_CASE("Seed phrase generation test", "[mnemonic][entropy]") {
auto test_data = core_test_utils::get_vector_tests();

REQUIRE_FALSE(test_data.empty());

MnemonicGenerator mnemgen;

for(auto& entry : test_data) {
    SECTION("Vector " + entry.seed_phrase.substr(0, 20) + "...") {
        bytes_data seed_phrase = mnemgen.__generateMnemonic(entry.entropy);
        REQUIRE(seed_phrase == bytes_data(entry.seed_phrase.begin(), entry.seed_phrase.end()));
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
