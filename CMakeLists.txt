cmake_minimum_required(VERSION 3.10)
project(EventDrivenBacktester CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

include_directories(include)

# ---- Main backtester engine ----
add_executable(backtester
    src/main.cpp
    src/DataHandler.cpp
    src/Strategy.cpp
    src/Portfolio.cpp
    src/ExecutionHandler.cpp
    src/PerformanceMetrics.cpp
)

# ---- Sample data generator (no external deps) ----
add_executable(generate_sample_data
    tools/generate_sample_data.cpp
)

# ---- Optional real-data fetcher (requires libcurl) ----
find_package(CURL)
if(CURL_FOUND)
    add_executable(fetch_data tools/fetch_data.cpp)
    target_link_libraries(fetch_data PRIVATE CURL::libcurl)
else()
    message(STATUS "libcurl not found - skipping fetch_data target (optional, see tools/fetch_data.cpp)")
endif()
