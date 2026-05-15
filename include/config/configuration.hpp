#pragma once

// PARAGRAPH
static constexpr auto MIN_Y = 3;

// BLOCKCHAIN
static constexpr auto MIN_API =
    "0f5e105675e98c45f556aa3ca75670e2bf0155ea3178fcb40e5506ce4f7e3ecc";

inline constexpr auto MIN_URL_BATCH =
    "https://min-api.cryptocompare.com/data/pricemulti?fsyms=";

// TIMEOUTS
static constexpr auto BALANCE_TIMEOUT = 10000;
static constexpr auto TRANSACTION_TIMEOUT = 10000;
static constexpr auto GWEI_TIMEOUT = 10000;
static constexpr auto FULL_UPDATE_TIMEOUT = 10000;

// WEI
static constexpr auto WEI_TO_ETH = "1000000000000000000";
static constexpr auto WEI_TO_GWEI = "1000000000";
