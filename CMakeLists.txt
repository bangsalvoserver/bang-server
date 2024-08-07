cmake_minimum_required(VERSION 3.13.0)
project(bangcardgame VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
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

# threads

find_package(Threads REQUIRED)
target_link_libraries(banglibs INTERFACE Threads::Threads)

# asio

target_compile_definitions(banglibs INTERFACE ASIO_STANDALONE)

if(MINGW)
    target_link_libraries(banglibs INTERFACE wsock32 ws2_32)
endif()

# openssl

find_package(OpenSSL)
if (NOT OPENSSL_FOUND)
    message(WARNING "Could not find OpenSSL")
    target_compile_definitions(banglibs INTERFACE WSSERVER_NO_TLS)
else()
    target_link_libraries(banglibs INTERFACE OpenSSL::SSL OpenSSL::Crypto)
endif()

# websocketpp

target_include_directories(banglibs INTERFACE external/websocketpp)
target_compile_definitions(banglibs INTERFACE _WEBSOCKETPP_CPP11_STL_)
target_compile_definitions(banglibs INTERFACE _WEBSOCKETPP_CPP11_THREAD_)

# range-v3

find_package(range-v3 REQUIRED)
target_link_libraries(banglibs INTERFACE range-v3)

# cxxopts

find_package(cxxopts REQUIRED)
target_link_libraries(banglibs INTERFACE cxxopts::cxxopts)

# nlohmann-json

find_package(nlohmann_json REQUIRED)
target_link_libraries(banglibs INTERFACE nlohmann_json::nlohmann_json)

# reflect

target_include_directories(banglibs INTERFACE external/reflect)
target_compile_definitions(banglibs INTERFACE NTEST)

# bang server executable

add_executable(bangserver "")

target_include_directories(bangserver PRIVATE src)
target_link_libraries(bangserver PRIVATE banglibs)

add_subdirectory(src)