cmake_minimum_required(VERSION 3.20)

project(wallet-src)

set(CORE_SOURCES
core/uint256.cpp
core/wallet.cpp
core/derive.cpp
core/security.cpp
core/mnemonic.cpp
)

set(UI_SOURCES
ui/cli.cpp
ui/config_cli.cpp
ui/ui.cpp
)

set(UTILS_SOURCES
utils/crypto_utils.cpp
utils/tech_utils.cpp
)
set(MANAGERS_SOURCES
async-managers/async_eth_usd_update.cpp
async-managers/async_balance.cpp
async-managers/async_transactions_history_manager.cpp
)

set(API_SOURCES
api/json.cpp
api/http.cpp
)

set(DRIVERS_SOURCES
drivers/blockchain_client.cpp
)

set(CONFIG_SOURCES
config/config.cpp
)

add_library(wallet_core STATIC ${CORE_SOURCES} ${UI_SOURCES} ${UTILS_SOURCES} ${MANAGERS_SOURCES}
${API_SOURCES} ${DRIVERS_SOURCES} ${CONFIG_SOURCES})

target_include_directories(wallet_core PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/ui ${CMAKE_SOURCE_DIR}/include/ui/ftxui-components ${CMAKE_SOURCE_DIR}/include/async-managers
${CMAKE_SOURCE_DIR}/include/utils ${CMAKE_SOURCE_DIR}/include/api ${CMAKE_SOURCE_DIR}/include/core ${CMAKE_SOURCE_DIR}/include/config ${CMAKE_SOURCE_DIR}/include/drivers ${SECP256K1_INCLUDE_DIRS})

target_link_libraries(wallet_core PUBLIC OpenSSL::Crypto 
nlohmann_json::nlohmann_json libcurl ftxui::screen
    ftxui::dom
    ftxui::component fmt::fmt secp256k1 tools)


add_executable(eth-wallet main.cpp)
target_link_libraries(eth-wallet PRIVATE wallet_core)