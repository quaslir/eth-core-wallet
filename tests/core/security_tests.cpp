#include <catch2/catch_all.hpp>
#include "catch2/catch_test_macros.hpp"
#include "config/config.hpp"
#include "core/wallet.hpp"
#include "core/security.hpp"
#include "utils/crypto_utils.hpp"

TEST_CASE("Testing wallet saving into a secure keystore", "[wallet][security]") {
Wallet wallet;
Config config;
bytes_data mnemonic = wallet.prepare_mnemonic(config);

const std::vector<uint32_t> PATH_DERIVE =
    Key_Derive::parse_derive_path(config.derivation_path);
bytes_data passphrase;
wallet.finalize_from_mnemonic(mnemonic, passphrase, PATH_DERIVE);


 bytes_data password = {1, 2, 3};

 // CHECKING SAVING DATA INTO A SECURE KEYSTORE
 REQUIRE(security_manager::first_time_save(wallet, password));

// CHECKING WALLET LOADING FROM KEYSTORE WITH INCORRECT PASSWORD

bytes_data incorrect_password = {1, 2};
REQUIRE_FALSE(security_manager::load_wallet(wallet, incorrect_password));

// CHECKING WALLET LOADING FROM KEYSTORE WITH CORRECT PASSWORD

REQUIRE(security_manager::load_wallet(wallet, password));
}


TEST_CASE("Incorrect filename saving", "[security][filesystem]") {
    Wallet wallet;
    Config config;
    bytes_data mnemonic = wallet.prepare_mnemonic(config);

    const std::vector<uint32_t> PATH_DERIVE =
        Key_Derive::parse_derive_path(config.derivation_path);
    bytes_data passphrase;
    wallet.finalize_from_mnemonic(mnemonic, passphrase, PATH_DERIVE);
    bytes_data password = {0};

      REQUIRE(security_manager::first_time_save(wallet, password, "save.json"));

     REQUIRE_FALSE(security_manager::load_wallet(wallet, password, "incorrect_save.json"));

     REQUIRE(security_manager::load_wallet(wallet, password, "save.json"));
}
