cmake_minimum_required(VERSION 3.20)

project(eth-core-wallet)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CORE_SOURCES
src/core/uint256.cpp
src/core/wallet.cpp
src/core/derive.cpp
src/core/security.cpp
src/core/mnemonic.cpp
)

set(UI_SOURCES
src/ui/cli.cpp
src/ui/config_cli.cpp
src/ui/ui.cpp
)

set(UTILS_SOURCES
src/utils/crypto_utils.cpp
src/utils/tech_utils.cpp
)
set(MANAGERS_SOURCES
src/async-managers/async_eth_usd_update.cpp
src/async-managers/async_balance.cpp
src/async-managers/async_transactions_history_manager.cpp
)

set(API_SOURCES
src/api/json.cpp
src/api/http.cpp
)

set(DRIVERS_SOURCES
src/drivers/blockchain_client.cpp
)

set(CONFIG_SOURCES
src/config/config.cpp
)

set(CMAKE_CXX_EXTENSIONS OFF)
include(FetchContent)

add_executable(${PROJECT_NAME} src/main.cpp ${CORE_SOURCES} ${UI_SOURCES} ${UTILS_SOURCES} ${MANAGERS_SOURCES}
${API_SOURCES} ${DRIVERS_SOURCES} ${CONFIG_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE include include/ui include/ui/ftxui-components include/async-managers
include/utils include/api include/core include/config include/drivers ${SECP256K1_INCLUDE_DIRS})

if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic -Werror -g
        -Wshadow -Wnon-virtual-dtor -Wold-style-cast
        -Wcast-align -Wunused-parameter
        -fstack-protector-strong
        -fsanitize=address -fsanitize=undefined
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        -fsanitize=address -fsanitize=undefined
    )

find_program(CPPCHECK_EXE NAMES "cppcheck")

if(CPPCHECK_EXE)
    file(GLOB_RECURSE CPPCHECK_SOURCES "src/*.cpp" "include/*.hpp")

    add_custom_target(cppcheck
        COMMAND ${CPPCHECK_EXE}
            --enable=all
            --std=c++20
            --inconclusive
            --suppress=missingIncludeSystem
            --inline-suppr
            -I ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CPPCHECK_SOURCES}
        COMMENT "Running static analysis with cppcheck..."
    )
endif()

find_program(CLANGTIDY_EXE NAMES "clang-tidy")


endif()

file(GLOB_RECURSE ALL_FILES "src/*.cpp" "include/*.hpp")
find_program(CLANG_FORMAT_EXE NAMES "clang-format")

if(CLANG_FORMAT_EXE)

add_custom_target(format
COMMAND ${CLANG_FORMAT_EXE} -i -style=LLVM ${ALL_FILES}
COMMENT "Formatting all source files with clang format..."
)
endif()

find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::Crypto ${SECP256K1_LDFLAGS})

add_subdirectory(external)
target_link_libraries(${PROJECT_NAME} PRIVATE tools)

set(HTTP_ONLY ON CACHE INTERNAL "")
set(CURL_DISABLE_FTP ON CACHE INTERNAL "")
set(CURL_DISABLE_TFTP ON CACHE INTERNAL "")
set(CURL_DISABLE_PROXY ON CACHE INTERNAL "")
set(CURL_DISABLE_DICT ON CACHE INTERNAL "")
set(CURL_DISABLE_FILE ON CACHE INTERNAL "")
set(CURL_DISABLE_GOPHER ON CACHE INTERNAL "")
set(CURL_DISABLE_SMTP ON CACHE INTERNAL "")
set(CURL_DISABLE_POP3 ON CACHE INTERNAL "")
set(CURL_DISABLE_IMAP ON CACHE INTERNAL "")
set(CURL_DISABLE_RTSP ON CACHE INTERNAL "")
set(CURL_DISABLE_TELNET ON CACHE INTERNAL "")
set(CURL_DISABLE_MQTT ON CACHE INTERNAL "")
set(ENABLE_THREADED_RESOLVER OFF CACHE INTERNAL "")
set(BUILD_TESTING OFF CACHE INTERNAL "")


FetchContent_Declare(
    json
    URL  https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)

FetchContent_Declare(
    curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG        curl-8_7_1
)

FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG v5.0.0
)


FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG      10.2.1
)

FetchContent_Declare(
  secp256k1
  GIT_REPOSITORY https://github.com/bitcoin-core/secp256k1.git
  GIT_TAG        v0.4.1
)


set(SECP256K1_ENABLE_MODULE_RECOVERY ON CACHE BOOL "" FORCE)
set(SECP256K1_ENABLE_MODULE_SCHNORRSIG OFF CACHE BOOL "" FORCE)
set(SECP256K1_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(curl)
FetchContent_MakeAvailable(ftxui)
FetchContent_MakeAvailable(fmt)
FetchContent_MakeAvailable(secp256k1)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json libcurl ftxui::screen
    ftxui::dom
    ftxui::component fmt::fmt secp256k1)
