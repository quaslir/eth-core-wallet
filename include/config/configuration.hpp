#pragma once

// PARAGRAPH
#define MIN_Y 3

// BLOCKCHAIN
#define MIN_API                                                                \
  "0f5e105675e98c45f556aa3ca75670e2bf0155ea3178fcb40e5506ce4f7e3ecc"

inline constexpr auto MIN_URL_BATCH =
    "https://min-api.cryptocompare.com/data/pricemulti?fsyms=";

// TIMEOUTS
#define BALANCE_TIMEOUT 10000
#define TRANSACTION_TIMEOUT 10000
#define GWEI_TIMEOUT 5000

// WEI
#define WEI_TO_ETH "1000000000000000000"
#define WEI_TO_GWEI "1000000000"
