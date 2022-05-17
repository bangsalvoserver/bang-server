cmake_minimum_required(VERSION 3.6.0)
project(bangcardgame VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(banggame_srcs
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
    src/game/make_all_cards.cpp
    src/game/draw_check_handler.cpp
    src/game/player.cpp
    src/game/play_verify.cpp
    src/game/possible_to_play.cpp
    src/game/manager.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(jsoncpp REQUIRED jsoncpp IMPORTED_TARGET)

add_subdirectory(external/cpputils)
add_subdirectory(resources)

add_library(networking INTERFACE)
target_include_directories(networking INTERFACE external/websocketpp)
target_compile_definitions(networking INTERFACE _WEBSOCKETPP_CPP11_THREAD_)
if(MINGW)
    target_link_libraries(networking INTERFACE wsock32 ws2_32)
endif()

add_library(banggame SHARED ${banggame_srcs})
target_link_libraries(banggame PUBLIC cpputils bang_cards_json PkgConfig::jsoncpp)
target_include_directories(banggame PUBLIC src)

option(DEBUG_PRINT_GAME_UPDATES "Debug Option: Print Game Updates" OFF)
if (DEBUG_PRINT_GAME_UPDATES)
    target_compile_definitions(banggame PUBLIC DEBUG_PRINT_GAME_UPDATES)
endif()

option(TESTING_CARDS "Debug Option: Enable Testing Cards" OFF)
if (TESTING_CARDS)
    target_compile_definitions(banggame PUBLIC TESTING_CARDS)
endif()

add_executable(bangserver src/net/main.cpp)
target_link_libraries(bangserver networking banggame)

install(TARGETS banggame bangserver)