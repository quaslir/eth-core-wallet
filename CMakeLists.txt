cmake_minimum_required(VERSION 3.20)

project(eth-core-wallet)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force static building" FORCE)
enable_testing()
include(FetchContent)
include(ExternalProject)
set(VENDOR_DIR ${CMAKE_BINARY_DIR}/vendor)
set(OPENSSL_INSTALL_DIR ${VENDOR_DIR}/openssl)
set(CURL_INSTALL_DIR ${VENDOR_DIR}/curl)
file(MAKE_DIRECTORY ${OPENSSL_INSTALL_DIR}/include)
file(MAKE_DIRECTORY ${CURL_INSTALL_DIR}/include)

ExternalProject_Add(ext_openssl
    URL https://github.com/openssl/openssl/releases/download/openssl-3.0.13/openssl-3.0.13.tar.gz
    PREFIX ${VENDOR_DIR}/openssl_build
    CONFIGURE_COMMAND <SOURCE_DIR>/config no-shared no-tests --prefix=${OPENSSL_INSTALL_DIR} --libdir=lib
    BUILD_COMMAND make -j8
    INSTALL_COMMAND make install_sw
)

add_library(OpenSSL::Crypto STATIC IMPORTED GLOBAL)
set_target_properties(OpenSSL::Crypto PROPERTIES
    IMPORTED_LOCATION ${OPENSSL_INSTALL_DIR}/lib/libcrypto.a
    INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INSTALL_DIR}/include
)
add_dependencies(OpenSSL::Crypto ext_openssl)

add_library(OpenSSL::SSL STATIC IMPORTED GLOBAL)
set_target_properties(OpenSSL::SSL PROPERTIES
    IMPORTED_LOCATION ${OPENSSL_INSTALL_DIR}/lib/libssl.a
    INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INSTALL_DIR}/include
    INTERFACE_LINK_LIBRARIES OpenSSL::Crypto
)
add_dependencies(OpenSSL::SSL ext_openssl)

ExternalProject_Add(ext_curl
    URL https://github.com/curl/curl/releases/download/curl-8_7_1/curl-8.7.1.tar.gz
    PREFIX ${VENDOR_DIR}/curl_build
    DEPENDS ext_openssl
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CURL_INSTALL_DIR}
        -DBUILD_SHARED_LIBS=OFF
        -DCURL_STATICLIB=ON
        -DCURL_USE_OPENSSL=ON
        -DOPENSSL_ROOT_DIR=${OPENSSL_INSTALL_DIR}
        -DOPENSSL_USE_STATIC_LIBS=TRUE
        -DBUILD_CURL_EXE=OFF
        -DHTTP_ONLY=ON
        -DCMAKE_USE_LIBSSH2=OFF
        -DCURL_USE_LIBSSH2=OFF
        -DCURL_USE_LIBSSH=OFF
        -DCURL_USE_LIBPSL=OFF
        -DCURL_BROTLI=OFF
        -DCURL_ZSTD=OFF
        -DCURL_USE_GSSAPI=OFF
        -DCURL_DISABLE_LDAP=ON
)

add_library(libcurl STATIC IMPORTED GLOBAL)
set_target_properties(libcurl PROPERTIES
    IMPORTED_LOCATION ${CURL_INSTALL_DIR}/lib/libcurl.a
    INTERFACE_INCLUDE_DIRECTORIES ${CURL_INSTALL_DIR}/include
    INTERFACE_LINK_LIBRARIES "OpenSSL::SSL;OpenSSL::Crypto"
)
add_dependencies(libcurl ext_curl)


FetchContent_Declare(
    json
    URL  https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
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

FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.15.3
)


set(SECP256K1_ENABLE_MODULE_RECOVERY ON CACHE BOOL "" FORCE)
set(SECP256K1_ENABLE_MODULE_SCHNORRSIG OFF CACHE BOOL "" FORCE)
set(SECP256K1_INSTALL OFF CACHE BOOL "" FORCE)


FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(ftxui)
FetchContent_MakeAvailable(fmt)
FetchContent_MakeAvailable(secp256k1)
FetchContent_MakeAvailable(httplib)

add_subdirectory(external)
add_subdirectory(src)



if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
endif()

if(TARGET wallet_core)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
target_compile_options(wallet_core PRIVATE
        -Wall -Wextra -Wpedantic -Werror -g
        -Wshadow -Wnon-virtual-dtor -Wold-style-cast
        -Wcast-align -Wunused-parameter
        -fstack-protector-strong
        -fsanitize=address -fsanitize=undefined
    )
    target_link_options(wallet_core PUBLIC
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
endif()

file(GLOB_RECURSE ALL_FILES "src/*.cpp" "include/*.hpp")
find_program(CLANG_FORMAT_EXE NAMES "clang-format")

if(CLANG_FORMAT_EXE)

add_custom_target(format
COMMAND ${CLANG_FORMAT_EXE} -i -style=LLVM ${ALL_FILES}
COMMENT "Formatting all source files with clang format..."
)
endif()


add_subdirectory(tests)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/tests/core/bip39-tests.json"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
