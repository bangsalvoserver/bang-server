cmake_minimum_required(VERSION 3.13.0)
project(bangcardgame VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/cpputils)

find_package(Threads REQUIRED)
find_package(cxxopts REQUIRED)

add_library(networking INTERFACE)
target_link_libraries(networking INTERFACE asio Threads::Threads)
target_compile_definitions(networking INTERFACE _WEBSOCKETPP_CPP11_STL_)
target_compile_definitions(networking INTERFACE _WEBSOCKETPP_CPP11_THREAD_)
target_include_directories(networking INTERFACE external/websocketpp)

set(bang_cards_cpp "${CMAKE_CURRENT_BINARY_DIR}/bang_cards.cpp")
add_custom_command(
    OUTPUT "${bang_cards_cpp}"
    COMMAND python3 "${CMAKE_CURRENT_SOURCE_DIR}/src/cards/parse_bang_cards.py"
        src/cards/bang_cards.yml
        "${bang_cards_cpp}"
    VERBATIM
    DEPENDS
        src/cards/bang_cards.yml
        "${CMAKE_CURRENT_SOURCE_DIR}/src/cards/parse_bang_cards.py"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
add_library(bang_cards_obj OBJECT "${bang_cards_cpp}")
target_include_directories(bang_cards_obj PRIVATE src)
target_link_libraries(bang_cards_obj PRIVATE cpputils)
if(MINGW)
    # disable optimization, fix ICE in gcc 13 in mingw
    target_compile_options(bang_cards_obj PRIVATE -O0)
endif()

add_library(bangcommon INTERFACE)
target_include_directories(bangcommon INTERFACE src)
target_link_libraries(bangcommon INTERFACE cpputils networking)

add_executable(bangserver "")
add_subdirectory(src)

target_link_libraries(bangserver PUBLIC bang_cards_obj bangcommon cxxopts::cxxopts)
target_compile_definitions(bangserver PRIVATE BUILD_BANG_SERVER)

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