cmake_minimum_required(VERSION 3.6.0)
project(bangcardgame VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(bangserver_srcs
    src/game/effects/base/bang.cpp
    src/game/effects/base/barrel.cpp
    src/game/effects/base/beer.cpp
    src/game/effects/base/black_jack.cpp
    src/game/effects/base/boots.cpp
    src/game/effects/base/calamity_janet.cpp
    src/game/effects/base/deathsave.cpp
    src/game/effects/base/draw.cpp
    src/game/effects/base/duel.cpp
    src/game/effects/base/dynamite.cpp
    src/game/effects/base/el_gringo.cpp
    src/game/effects/base/generalstore.cpp
    src/game/effects/base/heal.cpp
    src/game/effects/base/horseshoe.cpp
    src/game/effects/base/indians.cpp
    src/game/effects/base/initialcards.cpp
    src/game/effects/base/jail.cpp
    src/game/effects/base/kit_carlson.cpp
    src/game/effects/base/max_usages.cpp
    src/game/effects/base/missed.cpp
    src/game/effects/base/mustang.cpp
    src/game/effects/base/pass_turn.cpp
    src/game/effects/base/play_card_action.cpp
    src/game/effects/base/predraw_check.cpp
    src/game/effects/base/prompt_on_self_equip.cpp
    src/game/effects/base/requests.cpp
    src/game/effects/base/resolve.cpp
    src/game/effects/base/scope.cpp
    src/game/effects/base/self_damage.cpp
    src/game/effects/base/slab_the_killer.cpp
    src/game/effects/base/startofturn.cpp
    src/game/effects/base/steal_destroy.cpp
    src/game/effects/base/suzy_lafayette.cpp
    src/game/effects/base/volcanic.cpp
    src/game/effects/base/vulture_sam.cpp
    src/game/effects/base/weapon.cpp

    src/game/effects/dodgecity/bellestar.cpp
    src/game/effects/dodgecity/bill_noface.cpp
    src/game/effects/dodgecity/claus_the_saint.cpp
    src/game/effects/dodgecity/doc_holyday.cpp
    src/game/effects/dodgecity/greg_digger.cpp
    src/game/effects/dodgecity/herb_hunter.cpp
    src/game/effects/dodgecity/johnny_kisch.cpp
    src/game/effects/dodgecity/molly_stark.cpp
    src/game/effects/dodgecity/tequila_joe.cpp
    src/game/effects/dodgecity/vera_custer.cpp
    
    src/game/effects/highnoon/blessing.cpp
    src/game/effects/highnoon/curse.cpp
    src/game/effects/highnoon/ghosttown.cpp
    src/game/effects/highnoon/handcuffs.cpp
    src/game/effects/highnoon/hangover.cpp
    src/game/effects/highnoon/highnoon.cpp
    src/game/effects/highnoon/invert_rotation.cpp
    src/game/effects/highnoon/newidentity.cpp
    src/game/effects/highnoon/reverend.cpp
    src/game/effects/highnoon/sermon.cpp
    src/game/effects/highnoon/shootout.cpp
    src/game/effects/highnoon/thedaltons.cpp
    src/game/effects/highnoon/thedoctor.cpp
    src/game/effects/highnoon/thirst.cpp
    src/game/effects/highnoon/trainarrival.cpp

    src/game/effects/fistfulofcards/abandonedmine.cpp
    src/game/effects/fistfulofcards/ambush.cpp
    src/game/effects/fistfulofcards/deadman.cpp
    src/game/effects/fistfulofcards/fistfulofcards.cpp
    src/game/effects/fistfulofcards/judge.cpp
    src/game/effects/fistfulofcards/lasso.cpp
    src/game/effects/fistfulofcards/lawofthewest.cpp
    src/game/effects/fistfulofcards/peyote.cpp
    src/game/effects/fistfulofcards/ranch.cpp
    src/game/effects/fistfulofcards/ricochet.cpp
    src/game/effects/fistfulofcards/russianroulette.cpp
    src/game/effects/fistfulofcards/sniper.cpp
    src/game/effects/fistfulofcards/vendetta.cpp

    src/game/effects/goldrush/add_gold.cpp
    src/game/effects/goldrush/calumet.cpp
    src/game/effects/goldrush/discard_black.cpp
    src/game/effects/goldrush/don_bell.cpp
    src/game/effects/goldrush/dutch_will.cpp
    src/game/effects/goldrush/goldrush.cpp
    src/game/effects/goldrush/gunbelt.cpp
    src/game/effects/goldrush/josh_mccloud.cpp
    src/game/effects/goldrush/luckycharm.cpp
    src/game/effects/goldrush/madam_yto.cpp
    src/game/effects/goldrush/pay_gold.cpp
    src/game/effects/goldrush/pickaxe.cpp
    src/game/effects/goldrush/ruleset.cpp
    src/game/effects/goldrush/rum.cpp
    src/game/effects/goldrush/sell_beer.cpp
    src/game/effects/goldrush/wanted.cpp

    src/game/effects/valleyofshadows/aim.cpp
    src/game/effects/valleyofshadows/backfire.cpp
    src/game/effects/valleyofshadows/bandidos.cpp
    src/game/effects/valleyofshadows/bounty.cpp
    src/game/effects/valleyofshadows/colorado_bill.cpp
    src/game/effects/valleyofshadows/escape.cpp
    src/game/effects/valleyofshadows/evelyn_shebang.cpp
    src/game/effects/valleyofshadows/fanning.cpp
    src/game/effects/valleyofshadows/ghost.cpp
    src/game/effects/valleyofshadows/henry_block.cpp
    src/game/effects/valleyofshadows/lemonade_jim.cpp
    src/game/effects/valleyofshadows/play_as_gatling.cpp
    src/game/effects/valleyofshadows/poker.cpp
    src/game/effects/valleyofshadows/ruleset.cpp
    src/game/effects/valleyofshadows/saved.cpp
    src/game/effects/valleyofshadows/shotgun.cpp
    src/game/effects/valleyofshadows/snake.cpp
    src/game/effects/valleyofshadows/tornado.cpp
    src/game/effects/valleyofshadows/tuco_franziskaner.cpp

    src/game/effects/wildwestshow/big_spencer.cpp
    src/game/effects/wildwestshow/gary_looter.cpp
    src/game/effects/wildwestshow/flint_westwood.cpp
    src/game/effects/wildwestshow/gary_looter.cpp
    src/game/effects/wildwestshow/greygory_deck.cpp
    src/game/effects/wildwestshow/john_pain.cpp
    src/game/effects/wildwestshow/teren_kill.cpp
    src/game/effects/wildwestshow/youl_grinner.cpp

    src/game/effects/armedanddangerous/add_cube.cpp
    src/game/effects/armedanddangerous/al_preacher.cpp
    src/game/effects/armedanddangerous/bandolier.cpp
    src/game/effects/armedanddangerous/bigfifty.cpp
    src/game/effects/armedanddangerous/bloody_mary.cpp
    src/game/effects/armedanddangerous/bomb.cpp
    src/game/effects/armedanddangerous/buntlinespecial.cpp
    src/game/effects/armedanddangerous/doublebarrel.cpp
    src/game/effects/armedanddangerous/duck.cpp
    src/game/effects/armedanddangerous/flintlock.cpp
    src/game/effects/armedanddangerous/frankie_canton.cpp
    src/game/effects/armedanddangerous/julie_cutter.cpp
    src/game/effects/armedanddangerous/ms_abigail.cpp
    src/game/effects/armedanddangerous/red_ringo.cpp
    src/game/effects/armedanddangerous/ruleset.cpp
    src/game/effects/armedanddangerous/rust.cpp
    src/game/effects/armedanddangerous/squaw.cpp
    src/game/effects/armedanddangerous/thunderer.cpp
    src/game/effects/armedanddangerous/tumbleweed.cpp

    src/game/effects/canyondiablo/bronco.cpp
    src/game/effects/canyondiablo/brothel.cpp
    src/game/effects/canyondiablo/card_sharper.cpp
    src/game/effects/canyondiablo/disarm.cpp
    src/game/effects/canyondiablo/graverobber.cpp
    src/game/effects/canyondiablo/indianguide.cpp
    src/game/effects/canyondiablo/lastwill.cpp
    src/game/effects/canyondiablo/mirage.cpp
    src/game/effects/canyondiablo/packmule.cpp
    src/game/effects/canyondiablo/ruleset.cpp
    src/game/effects/canyondiablo/sacrifice.cpp
    src/game/effects/canyondiablo/taxman.cpp

    src/game/card_serial.cpp
    src/game/filters.cpp
    src/game/holders.cpp
    src/game/game.cpp
    src/game/game_table.cpp
    src/game/draw_check_handler.cpp
    src/game/player.cpp
    src/game/player_iterator.cpp
    src/game/play_verify.cpp
    src/game/play_visitor.cpp
    src/game/possible_to_play.cpp
    src/game/manager.cpp
    src/net/main.cpp
)

add_subdirectory(external/cpputils)

find_package(Threads REQUIRED)

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

add_library(bangcommon INTERFACE)
target_include_directories(bangcommon INTERFACE src)
target_link_libraries(bangcommon INTERFACE cpputils networking)

add_executable(bangserver ${bangserver_srcs})
target_link_libraries(bangserver PUBLIC bang_cards_obj bangcommon)
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

else()

    option(TESTING_CARDS "Debug Option: Enable Testing Cards" ON)
    if (TESTING_CARDS)
        target_compile_definitions(bangserver PUBLIC TESTING_CARDS)
    endif()

endif()