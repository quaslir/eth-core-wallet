cmake_minimum_required(VERSION 3.20)

project(external_libs)

add_library(tools STATIC src/Keccak256.cpp src/eip712.cpp)


target_link_libraries(tools PUBLIC nlohmann_json::nlohmann_json)
target_include_directories(tools PUBLIC include)
