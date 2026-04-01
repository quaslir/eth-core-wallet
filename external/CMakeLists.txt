cmake_minimum_required(VERSION 3.20)

project(external_libs)

add_library(tools STATIC src/Keccak256.cpp)

target_include_directories(tools PUBLIC include)