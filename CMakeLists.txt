cmake_minimum_required(VERSION 3.13.0)
project(bangcardgame VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    if (MSVC)
        add_compile_options(/bigobj)
        add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    elseif(MINGW)
        add_compile_options(-fno-ms-extensions)
    endif()
    add_compile_definitions(_WIN32_WINNT=0x0601)
endif()

add_subdirectory(external/cpputils)

find_package(Threads REQUIRED)
find_package(cxxopts REQUIRED)

add_library(networking INTERFACE)
target_link_libraries(networking INTERFACE Threads::Threads)
target_compile_definitions(networking INTERFACE _WEBSOCKETPP_CPP11_STL_)
target_compile_definitions(networking INTERFACE _WEBSOCKETPP_CPP11_THREAD_)
target_compile_definitions(networking INTERFACE ASIO_STANDALONE)
target_include_directories(networking INTERFACE external/websocketpp)
target_include_directories(networking INTERFACE external/asio/asio/include)

if(MINGW)
    target_link_libraries(networking INTERFACE wsock32 ws2_32)
endif()

add_library(bangcommon INTERFACE)
target_include_directories(bangcommon INTERFACE src)
target_link_libraries(bangcommon INTERFACE cpputils networking)

add_executable(bangserver "")
add_subdirectory(src)

target_link_libraries(bangserver PUBLIC bangcommon cxxopts::cxxopts)

option(BUILD_GAME_RELEASE "Build Game Release" OFF)

if (BUILD_GAME_RELEASE)

    set(PRE_CONFIGURE_FILE .cmake/git_version.cpp.in)
    set(POST_CONFIGURE_FILE "${CMAKE_CURRENT_BINARY_DIR}/git_version.cpp")
    set(GIT_WORKING_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    set(CHECK_GIT_TARGET_NAME check_git_server)
    include(git_watcher.cmake)

    add_library(git_version STATIC "${POST_CONFIGURE_FILE}")
    add_dependencies(git_version check_git_server)
    target_compile_definitions(git_version PUBLIC HAVE_GIT_VERSION)

    target_link_libraries(bangcommon INTERFACE git_version)

endif()