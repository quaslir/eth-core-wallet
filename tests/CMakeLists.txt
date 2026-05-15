cmake_minimum_required(VERSION 3.20)
project(eth-core-wallet-tests)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)
include(Catch)

set(TEST_SOURCES
    core/test_mnemonic
    core/test_derivation
    core/security_tests
    api/test_rlp
)

foreach(TEST_SRC ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME)

    add_executable(${TEST_NAME} ${TEST_SRC}.cpp)

    target_link_libraries(${TEST_NAME} PRIVATE Catch2::Catch2WithMain wallet_core)
    target_compile_options(${TEST_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_definitions(${TEST_NAME} PRIVATE
        TEST_JSON_PATH="${CMAKE_BINARY_DIR}/bip39-tests.json"
    )

    catch_discover_tests(${TEST_NAME}
        PROPERTIES LABELS "MY_WALLET_TESTS"
    )
endforeach()
