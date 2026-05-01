#include <catch2/catch_all.hpp>
#include "catch2/catch_test_macros.hpp"
#include "config/config.hpp"
#include "core/secure_bytes_data.hpp"
#include "core/wallet.hpp"
#include "core/security.hpp"


TEST_CASE("Testing wallet saving into a secure keystore", "[wallet][security]") {
Wallet wallet;
Config config;
secure_string mnemonic = wallet.prepare_mnemonic(config);

const std::vector<uint32_t> PATH_DERIVE =
    Key_Derive::parse_derive_path(config.derivation_path);

wallet.finalize_from_mnemonic(mnemonic, secure_string{}, PATH_DERIVE);


 secure_string password = {1, 2, 3};

 // CHECKING SAVING DATA INTO A SECURE KEYSTORE
 REQUIRE(security_manager::first_time_save(wallet, password));

// CHECKING WALLET LOADING FROM KEYSTORE WITH INCORRECT PASSWORD

secure_string incorrect_password = {1, 2};
REQUIRE_FALSE(security_manager::load_wallet(wallet, incorrect_password));

// CHECKING WALLET LOADING FROM KEYSTORE WITH CORRECT PASSWORD

REQUIRE(security_manager::load_wallet(wallet, password));
}


TEST_CASE("Incorrect filename saving", "[security][filesystem]") {
    Wallet wallet;
    Config config;
    secure_string mnemonic = wallet.prepare_mnemonic(config);

    const std::vector<uint32_t> PATH_DERIVE =
        Key_Derive::parse_derive_path(config.derivation_path);
    wallet.finalize_from_mnemonic(mnemonic, secure_string{}, PATH_DERIVE);
    secure_string password = {0};

      REQUIRE(security_manager::first_time_save(wallet, password, "save.json"));

     REQUIRE_FALSE(security_manager::load_wallet(wallet, password, "incorrect_save.json"));

     REQUIRE(security_manager::load_wallet(wallet, password, "save.json"));
}
