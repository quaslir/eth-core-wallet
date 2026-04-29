cmake_minimum_required(VERSION 3.20)

project(eth-core-wallet-tests)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_executable(${PROJECT_NAME} test_main.cpp core/test_mnemonic.cpp)


include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
)

FetchContent_MakeAvailable(Catch2)



target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2WithMain wallet_core)
