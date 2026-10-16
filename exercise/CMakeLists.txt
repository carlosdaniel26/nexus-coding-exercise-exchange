cmake_minimum_required(VERSION 3.20)
project(exchange_client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(httplib REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(GTest REQUIRED)

add_library(exchange_core STATIC src/exchange.cpp)
target_include_directories(exchange_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(exchange_core PUBLIC nlohmann_json::nlohmann_json)
target_compile_options(exchange_core PRIVATE -fno-omit-frame-pointer)

add_executable(exchange_client src/main.cpp)
target_link_libraries(exchange_client PRIVATE exchange_core httplib::httplib)
target_compile_options(exchange_client PRIVATE -fno-omit-frame-pointer)

enable_testing()

add_executable(exchange_tests test/main_test.cpp)
target_link_libraries(exchange_tests PRIVATE exchange_core GTest::gtest_main)
target_compile_options(exchange_tests PRIVATE -fno-omit-frame-pointer)

include(GoogleTest)
gtest_discover_tests(exchange_tests)
