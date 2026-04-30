#include <catch2/catch_all.hpp>
#include <cstdint>
#include "api/json.hpp"
#include "catch2/catch_test_macros.hpp"
#include "core/wallet.hpp"
#include "utils/tech_utils.hpp"
#include "utils/utils.hpp"
#include "core/bip39.hpp"

TEST_CASE("Derivation from seed tests", "[derivaion]") {
    auto test_cases = core_test_utils::get_vector_tests();
    static const std::vector<uint32_t> PATH_DERIVE =
        Key_Derive::parse_derive_path("m/44'/60'/0'/0/0");
    for(auto& entry : test_cases ) {
        SECTION("Vector " + entry.seed_phrase.substr(0, 20) + "...") {
            Wallet wallet;
            bytes_data mnemonic(entry.seed_phrase.begin(), entry.seed_phrase.end());
            bytes_data passphrase = {};
            wallet.finalize_from_mnemonic(mnemonic, passphrase, PATH_DERIVE);
            REQUIRE(wallet.get_eth_address() == entry.first_eth_addr);
            std::string private_key = wallet.__get_private_key_hex();

            REQUIRE(private_key == entry.first_eth_private_key);
        }
    }

}
