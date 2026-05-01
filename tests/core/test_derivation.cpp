#include <catch2/catch_all.hpp>
#include <cstdint>
#include <string>
#include "api/json.hpp"
#include "catch2/catch_test_macros.hpp"
#include "core/secure_bytes_data.hpp"
#include "core/wallet.hpp"
#include "utils/tech_utils.hpp"
#include "utils/utils.hpp"
#include "core/bip39.hpp"

// CHANGE
TEST_CASE("Derivation from seed tests", "[derivaion]") {
    auto test_cases = core_test_utils::get_vector_tests();
    static const std::vector<uint32_t> PATH_DERIVE =
        Key_Derive::parse_derive_path("m/44'/60'/0'/0/0");
    for(auto& entry : test_cases ) {
        SECTION("Vector " + entry.seed_phrase.substr(0, 20) + "...") {
            Wallet wallet;
            secure_string mnemonic(entry.seed_phrase.begin(), entry.seed_phrase.end());

            wallet.finalize_from_mnemonic(mnemonic, secure_string{}, PATH_DERIVE);
            REQUIRE(std::string{wallet.get_eth_address()} == entry.first_eth_addr);
            secure_string private_key = wallet.__get_private_key_hex();

            REQUIRE(std::string{private_key} == entry.first_eth_private_key);
        }
    }

}
