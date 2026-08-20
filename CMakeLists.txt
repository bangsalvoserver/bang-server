cmake_minimum_required(VERSION 3.16.0)
project(bangcardgame VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

if(WIN32)
    if (MSVC)
        add_compile_options(/bigobj)
        add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
        add_compile_definitions(_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
    elseif(MINGW)
        add_compile_options(-fno-ms-extensions)
        add_compile_options(-Wa,-mbig-obj)
        link_libraries(stdc++exp ws2_32 iphlpapi userenv psapi)
    endif()
    add_compile_definitions(_WIN32_WINNT=0x0601)
endif()

# libuv (nutné pro uSockets na Windows)
FetchContent_Declare(
    libuv
    GIT_REPOSITORY https://github.com/libuv/libuv.git
    GIT_TAG v1.48.0
)
set(LIBUV_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(libuv)
include_directories(${libuv_SOURCE_DIR}/include)

# external dependencies

add_library(banglibs INTERFACE)

# uwebsockets

add_subdirectory(external/uwebsockets)
target_link_libraries(banglibs INTERFACE uwebsockets uv_a)

# reflect

add_subdirectory(external/reflect)
target_link_libraries(banglibs INTERFACE reflect)

# sqlite

find_package(SQLite3 QUIET)
if(NOT SQLite3_FOUND)
    FetchContent_Declare(
        sqlite3
        URL https://www.sqlite.org/2024/sqlite-amalgamation-3450300.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(sqlite3)
    add_library(sqlite3_lib STATIC ${sqlite3_SOURCE_DIR}/sqlite3.c)
    target_include_directories(sqlite3_lib PUBLIC ${sqlite3_SOURCE_DIR})
    add_library(SQLite::SQLite3 ALIAS sqlite3_lib)
endif()
target_link_libraries(banglibs INTERFACE SQLite::SQLite3)

# cxxopts

find_package(cxxopts QUIET)
if(NOT cxxopts_FOUND)
    FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG v3.2.0
    )
    FetchContent_MakeAvailable(cxxopts)
endif()
target_link_libraries(banglibs INTERFACE cxxopts::cxxopts)

# rapidjson

add_subdirectory(external/rapidjson)
target_link_libraries(banglibs INTERFACE RapidJSON)

# libpng & zlib

find_package(PNG QUIET)
if(NOT PNG_FOUND)
    FetchContent_Declare(
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG v1.3.1
    )
    FetchContent_MakeAvailable(zlib)
    
    set(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE PATH "" FORCE)
    set(ZLIB_LIBRARY "${CMAKE_BINARY_DIR}/_deps/zlib-build/libzlibstatic.a" CACHE FILEPATH "" FORCE)
    set(ZLIB_LIBRARIES "${CMAKE_BINARY_DIR}/_deps/zlib-build/libzlibstatic.a" CACHE FILEPATH "" FORCE)
    set(ZLIB_FOUND TRUE CACHE BOOL "" FORCE)

    FetchContent_Declare(
        libpng
        GIT_REPOSITORY https://github.com/pnggroup/libpng.git
        GIT_TAG v1.6.43
    )
    set(PNG_SHARED OFF CACHE BOOL "" FORCE)
    set(PNG_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(libpng)
    if(TARGET png_static)
        add_library(PNG::PNG ALIAS png_static)
    elseif(TARGET png)
        add_library(PNG::PNG ALIAS png)
    endif()
endif()
target_link_libraries(banglibs INTERFACE PNG::PNG)

# bang server executable

add_executable(bangserver "")

target_include_directories(bangserver PRIVATE src)
target_link_libraries(bangserver PRIVATE banglibs "${CMAKE_BINARY_DIR}/_deps/zlib-build/libzlibstatic.a")

target_precompile_headers(bangserver PRIVATE
    <ranges>
    <format>
    <variant>
    <chrono>
)

add_subdirectory(src)