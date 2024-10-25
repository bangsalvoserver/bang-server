cmake_minimum_required(VERSION 3.13.0)
project(bangcardgame VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    if (MSVC)
        add_compile_options(/bigobj)
        add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
        add_compile_definitions(_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
    elseif(MINGW)
        add_compile_options(-fno-ms-extensions)
    endif()
    add_compile_definitions(_WIN32_WINNT=0x0601)
endif()

# external dependencies

add_library(banglibs INTERFACE)

# uwebsockets

add_subdirectory(external/uwebsockets)
target_link_libraries(banglibs INTERFACE uwebsockets)

# reflect

add_subdirectory(external/reflect)
target_link_libraries(banglibs INTERFACE reflect)

# sqlite

find_package(SQLite3 REQUIRED)
target_link_libraries(banglibs INTERFACE SQLite::SQLite3)

# range-v3

find_package(range-v3 REQUIRED)
target_link_libraries(banglibs INTERFACE range-v3)

# cxxopts

find_package(cxxopts REQUIRED)
target_link_libraries(banglibs INTERFACE cxxopts::cxxopts)

# nlohmann-json

find_package(nlohmann_json 3.11.0 REQUIRED)
target_link_libraries(banglibs INTERFACE nlohmann_json::nlohmann_json)

# libpng

find_package(PNG REQUIRED)
target_link_libraries(banglibs INTERFACE PNG::PNG)

# bang server executable

add_executable(bangserver "")

target_include_directories(bangserver PRIVATE src)
target_link_libraries(bangserver PRIVATE banglibs)

add_subdirectory(src)