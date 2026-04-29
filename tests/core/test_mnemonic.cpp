#include <catch2/catch_all.hpp>
#include "catch2/internal/catch_section.hpp"
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




TEST_CASE("Mnemonic massive stress test", "[mnemonic][security][stress]") {
    MnemonicGenerator mnem_gen;

    auto invalid_phrase = GENERATE(
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon junk",
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon 12345",
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon !!!",
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon",
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon",
        "zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo",
        "void void void void void void void void void void void void",
        "apple apple apple apple apple apple apple apple apple apple apple apple",
        "  ",
        "\t\n\r",
        "abandon  abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
        " abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about ",
        "ABANDON ABANDON ABANDON ABANDON ABANDON ABANDON ABANDON ABANDON ABANDON ABANDON ABANDON ABOUT",
        "Abandon Abandon Abandon Abandon Abandon Abandon Abandon Abandon Abandon Abandon Abandon About",
        "abandoN abandoN abandoN abandoN abandoN abandoN abandoN abandoN abandoN abandoN abandoN abouT",
        "a b a n d o n a b a n d o n a b a n d o n a b a n d o n a b a n d o n a b a n d o n",
        "abandonabandonabandonabandonabandonabandonabandonabandonabandonabandonabandonabout",
        "abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,abandon,about",
        "abandon;abandon;abandon;abandon;abandon;abandon;abandon;abandon;abandon;abandon;abandon;about",
        "abandon.abandon.abandon.abandon.abandon.abandon.abandon.abandon.abandon.abandon.abandon.about",
        "abandon_abandon_abandon_abandon_abandon_abandon_abandon_abandon_abandon_abandon_abandon_about",
        "1 2 3 4 5 6 7 8 9 10 11 12",
        "яблоко яблоко яблоко яблоко яблоко яблоко яблоко яблоко яблоко яблоко яблоко яблоко",
        "你好 你好 你好 你好 你好 你好 你好 你好 你好 你好 你好 你好",
        "0x0x0x0x0x0x0x0x0x0x0x0x0x0x0x0x0x0x0x0x0x0x0x0x",
        "------------------------------------------------",
        "////////////////////////////////////////////////",
        "ffffffffffffffffffffffffffffffffffffffffffffffff",
        "NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN",
        "null null null null null null null null null null null null",
        "undefined undefined undefined undefined undefined undefined",
        "true true true true true true true true true true true true",
        "false false false false false false false false false false",
        "dog dog dog dog dog dog dog dog dog dog dog dog dog dog dog dog dog dog dog dog dog dog dog dog",
        "alpha beta gamma delta epsilon zeta eta theta iota kappa lambda mu",
        "one two three four five six seven eight nine ten eleven twelve",
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abou",
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon aboutx",
        "xabandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abou t",
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon a bout",
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abou\0t",
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon \xFF\xFE",
        "word-word-word-word-word-word-word-word-word-word-word-word",
        "abandon+abandon+abandon+abandon+abandon+abandon+abandon+abandon+abandon+abandon+abandon+about",
        "abandon*abandon*abandon*abandon*abandon*abandon*abandon*abandon*abandon*abandon*abandon*about",
        "abandon/abandon/abandon/abandon/abandon/abandon/abandon/abandon/abandon/abandon/abandon/about",
        "................................................................................",
        "         abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about         ",
        "abandon \t abandon \n abandon \r abandon abandon abandon abandon abandon abandon abandon abandon about",
        "abandon-abandon-abandon-abandon-abandon-abandon-abandon-abandon-abandon-abandon-abandon-about",
        "abandon. abandon. abandon. abandon. abandon. abandon. abandon. abandon. abandon. abandon. abandon. about.",
        "abandon! abandon! abandon! abandon! abandon! abandon! abandon! abandon! abandon! abandon! abandon! about!",
        "abandon? abandon? abandon? abandon? abandon? abandon? abandon? abandon? abandon? abandon? abandon? about?",
        "abandon:abandon:abandon:abandon:abandon:abandon:abandon:abandon:abandon:abandon:abandon:about",
        "abandon|abandon|abandon|abandon|abandon|abandon|abandon|abandon|abandon|abandon|abandon|about",
        "abandon~abandon~abandon~abandon~abandon~abandon~abandon~abandon~abandon~abandon~abandon~about",
        "abandon^abandon^abandon^abandon^abandon^abandon^abandon^abandon^abandon^abandon^abandon^about",
        "abandon#abandon#abandon#abandon#abandon#abandon#abandon#abandon#abandon#abandon#abandon#about",
        "abandon$abandon$abandon$abandon$abandon$abandon$abandon$abandon$abandon$abandon$abandon$about",
        "abandon%abandon%abandon%abandon%abandon%abandon%abandon%abandon%abandon%abandon%abandon%about",
        "abandon&abandon&abandon&abandon&abandon&abandon&abandon&abandon&abandon&abandon&abandon&about",
        "abandon(abandon)abandon(abandon)abandon(abandon)abandon(abandon)abandon(abandon)abandon(about)",
        "abandon[abandon]abandon[abandon]abandon[abandon]abandon[abandon]abandon[abandon]abandon[about]",
        "abandon{abandon}abandon{abandon}abandon{abandon}abandon{abandon}abandon{abandon}abandon{about}",
        "abandon<abandon>abandon<abandon>abandon<abandon>abandon<abandon>abandon<abandon>abandon<about>",
        "abandon'abandon'abandon'abandon'abandon'abandon'abandon'abandon'abandon'abandon'abandon'about",
        "abandon\"abandon\"abandon\"abandon\"abandon\"abandon\"abandon\"abandon\"abandon\"abandon\"abandon\"about",
        "abandon`abandon`abandon`abandon`abandon`abandon`abandon`abandon`abandon`abandon`abandon`about",
        "a b c d e f g h i j k l m n o p q r s t u v w x y z",
        "abandon\0",
        "abandon\1",
        "abandon\2",
        "abandon\3",
        "abandon\4",
        "abandon\5",
        "abandon\6",
        "abandon\7",
        "abandon\10",
        "abandon\11",
        "abandon\12",
        "abandon\13",
        "abandon\14",
        "abandon\15",
        "abandon\16",
        "abandon\17",
        "abandon\20",
        "abandon\21",
        "abandon\22",
        "abandon\23",
        "abandon\24",
        "abandon\25",
        "abandon\26",
        "abandon\27",
        "abandon\30",
        "abandon\31",
        "abandon\32"
    );

    REQUIRE_FALSE(mnem_gen.mnemonic_is_correct(invalid_phrase));
}
