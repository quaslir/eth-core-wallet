cmake_minimum_required(VERSION 3.20)

project(eth-core-wallet)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
src/main.cpp
src/mnemonic.cpp
src/utils.cpp
src/derive.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SECP256K1 REQUIRED libsecp256k1)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE include ${SECP256K1_INCLUDE_DIRS})

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