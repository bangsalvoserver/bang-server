cmake_minimum_required(VERSION 3.6.0)
project(bangcardgame VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(bangserver_srcs
    src/game/effects/base/effects.cpp
    src/game/effects/base/equips.cpp
    src/game/effects/base/characters.cpp
    src/game/effects/base/requests.cpp
    src/game/effects/dodgecity/characters.cpp
    src/game/effects/highnoon/scenarios.cpp
    src/game/effects/fistfulofcards/scenarios.cpp
    src/game/effects/goldrush/effects.cpp
    src/game/effects/goldrush/equips.cpp
    src/game/effects/goldrush/characters.cpp
    src/game/effects/valleyofshadows/effects.cpp
    src/game/effects/valleyofshadows/equips.cpp
    src/game/effects/valleyofshadows/characters.cpp
    src/game/effects/valleyofshadows/requests.cpp
    src/game/effects/wildwestshow/characters.cpp
    src/game/effects/armedanddangerous/equips.cpp
    src/game/effects/armedanddangerous/effects.cpp
    src/game/effects/armedanddangerous/characters.cpp
    src/game/effects/armedanddangerous/requests.cpp
    src/game/effects/canyondiablo/effects.cpp
    src/game/effects/canyondiablo/equips.cpp
    src/game/holders.cpp
    src/game/game.cpp
    src/game/game_table.cpp
    src/game/draw_check_handler.cpp
    src/game/player.cpp
    src/game/player_iterator.cpp
    src/game/play_verify.cpp
    src/game/possible_to_play.cpp
    src/game/manager.cpp
    src/net/main.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(jsoncpp REQUIRED jsoncpp IMPORTED_TARGET)
pkg_check_modules(yaml-cpp REQUIRED yaml-cpp IMPORTED_TARGET)

add_subdirectory(external/cpputils)

add_library(networking INTERFACE)
target_link_libraries(networking INTERFACE asio)
target_compile_definitions(networking INTERFACE _WEBSOCKETPP_CPP11_THREAD_)
target_include_directories(networking INTERFACE external/websocketpp)

add_executable(parse_bang_cards src/cards/parse_bang_cards.cpp)
target_link_libraries(parse_bang_cards PRIVATE cpputils PkgConfig::yaml-cpp)

set(bang_cards_cpp "${CMAKE_CURRENT_BINARY_DIR}/bang_cards.cpp")
add_custom_command(
    OUTPUT "${bang_cards_cpp}"
    COMMAND parse_bang_cards
        src/cards/bang_cards.yml
        "${bang_cards_cpp}"
    VERBATIM
    DEPENDS
        src/cards/bang_cards.yml
        parse_bang_cards
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
add_library(bang_cards_obj OBJECT "${bang_cards_cpp}")
target_include_directories(bang_cards_obj PRIVATE src)
target_link_libraries(bang_cards_obj PRIVATE cpputils)

add_library(bangcommon INTERFACE)
target_include_directories(bangcommon INTERFACE src)
target_link_libraries(bangcommon INTERFACE cpputils networking PkgConfig::jsoncpp)

add_executable(bangserver ${bangserver_srcs})
target_link_libraries(bangserver PUBLIC bang_cards_obj bangcommon)

option(DEBUG_PRINT_GAME_UPDATES "Debug Option: Print Game Updates" OFF)
if (DEBUG_PRINT_GAME_UPDATES)
    target_compile_definitions(bangserver PUBLIC DEBUG_PRINT_GAME_UPDATES)
endif()

option(TESTING_CARDS "Debug Option: Enable Testing Cards" OFF)
if (TESTING_CARDS)
    target_compile_definitions(bangserver PUBLIC TESTING_CARDS)
endif()

install(TARGETS bangserver)