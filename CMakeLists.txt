cmake_minimum_required(VERSION 3.14) 
project(order_matching_engine CXX)
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git GIT_TAG 1.11.2) 
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.11.3)
FetchContent_MakeAvailable(json)

find_package(Threads REQUIRED)

add_executable(viewer
src/viewer.cpp
src/order_book.cpp
src/binance.cpp
src/display.cpp)

target_link_libraries(viewer PRIVATE cpr::cpr nlohmann_json::nlohmann_json Threads::Threads)
target_include_directories(viewer PRIVATE src)

add_executable(trader src/trader.cpp)
target_include_directories(trader PRIVATE src)
