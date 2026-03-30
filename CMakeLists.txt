cmake_minimum_required(VERSION 3.20)

project(eth-core-wallet)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
src/main.cpp
src/mnemonic.cpp
src/utils.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE include)

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

set(CPPCHECK_COMMAND
${CPPCHECK_EXE}
"--enable=all"
"--suppress=missingIncludeSystem"
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CPPCHECK "${CPPCHECK_COMMAND}")

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
target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::Crypto)