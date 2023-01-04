cmake_minimum_required(VERSION 3.6.0)
project(bangcardgame VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(bangserver_srcs
    src/cards/base/bang.cpp
    src/cards/base/barrel.cpp
    src/cards/base/beer.cpp
    src/cards/base/black_jack.cpp
    src/cards/base/bart_cassidy.cpp
    src/cards/base/calamity_janet.cpp
    src/cards/base/damage.cpp
    src/cards/base/deathsave.cpp
    src/cards/base/draw.cpp
    src/cards/base/duel.cpp
    src/cards/base/dynamite.cpp
    src/cards/base/el_gringo.cpp
    src/cards/base/generalstore.cpp
    src/cards/base/heal.cpp
    src/cards/base/lucky_duke.cpp
    src/cards/base/indians.cpp
    src/cards/base/initialcards.cpp
    src/cards/base/jail.cpp
    src/cards/base/kit_carlson.cpp
    src/cards/base/max_usages.cpp
    src/cards/base/missed.cpp
    src/cards/base/mustang.cpp
    src/cards/base/pass_turn.cpp
    src/cards/base/play_card_action.cpp
    src/cards/base/predraw_check.cpp
    src/cards/base/prompt_on_self_equip.cpp
    src/cards/base/requests.cpp
    src/cards/base/resolve.cpp
    src/cards/base/scope.cpp
    src/cards/base/slab_the_killer.cpp
    src/cards/base/startofturn.cpp
    src/cards/base/steal_destroy.cpp
    src/cards/base/suzy_lafayette.cpp
    src/cards/base/volcanic.cpp
    src/cards/base/vulture_sam.cpp
    src/cards/base/weapon.cpp

    src/cards/dodgecity/apache_kid.cpp
    src/cards/dodgecity/bellestar.cpp
    src/cards/dodgecity/bill_noface.cpp
    src/cards/dodgecity/claus_the_saint.cpp
    src/cards/dodgecity/doc_holyday.cpp
    src/cards/dodgecity/greg_digger.cpp
    src/cards/dodgecity/herb_hunter.cpp
    src/cards/dodgecity/johnny_kisch.cpp
    src/cards/dodgecity/molly_stark.cpp
    src/cards/dodgecity/pixie_pete.cpp
    src/cards/dodgecity/tequila_joe.cpp
    src/cards/dodgecity/vera_custer.cpp
    
    src/cards/highnoon/blessing.cpp
    src/cards/highnoon/curse.cpp
    src/cards/highnoon/ghosttown.cpp
    src/cards/highnoon/handcuffs.cpp
    src/cards/highnoon/hangover.cpp
    src/cards/highnoon/highnoon.cpp
    src/cards/highnoon/invert_rotation.cpp
    src/cards/highnoon/newidentity.cpp
    src/cards/highnoon/reverend.cpp
    src/cards/highnoon/sermon.cpp
    src/cards/highnoon/shootout.cpp
    src/cards/highnoon/thedaltons.cpp
    src/cards/highnoon/thedoctor.cpp
    src/cards/highnoon/thirst.cpp
    src/cards/highnoon/trainarrival.cpp

    src/cards/fistfulofcards/abandonedmine.cpp
    src/cards/fistfulofcards/ambush.cpp
    src/cards/fistfulofcards/deadman.cpp
    src/cards/fistfulofcards/fistfulofcards.cpp
    src/cards/fistfulofcards/judge.cpp
    src/cards/fistfulofcards/lasso.cpp
    src/cards/fistfulofcards/lawofthewest.cpp
    src/cards/fistfulofcards/peyote.cpp
    src/cards/fistfulofcards/ranch.cpp
    src/cards/fistfulofcards/ricochet.cpp
    src/cards/fistfulofcards/russianroulette.cpp
    src/cards/fistfulofcards/sniper.cpp
    src/cards/fistfulofcards/vendetta.cpp

    src/cards/goldrush/add_gold.cpp
    src/cards/goldrush/discard_black.cpp
    src/cards/goldrush/don_bell.cpp
    src/cards/goldrush/dutch_will.cpp
    src/cards/goldrush/goldrush.cpp
    src/cards/goldrush/gunbelt.cpp
    src/cards/goldrush/josh_mccloud.cpp
    src/cards/goldrush/luckycharm.cpp
    src/cards/goldrush/madam_yto.cpp
    src/cards/goldrush/pay_gold.cpp
    src/cards/goldrush/ruleset.cpp
    src/cards/goldrush/rum.cpp
    src/cards/goldrush/sell_beer.cpp
    src/cards/goldrush/wanted.cpp

    src/cards/valleyofshadows/aim.cpp
    src/cards/valleyofshadows/backfire.cpp
    src/cards/valleyofshadows/bandidos.cpp
    src/cards/valleyofshadows/bounty.cpp
    src/cards/valleyofshadows/colorado_bill.cpp
    src/cards/valleyofshadows/escape.cpp
    src/cards/valleyofshadows/evelyn_shebang.cpp
    src/cards/valleyofshadows/ghost.cpp
    src/cards/valleyofshadows/henry_block.cpp
    src/cards/valleyofshadows/lemonade_jim.cpp
    src/cards/valleyofshadows/mick_defender.cpp
    src/cards/valleyofshadows/play_as_gatling.cpp
    src/cards/valleyofshadows/poker.cpp
    src/cards/valleyofshadows/ruleset.cpp
    src/cards/valleyofshadows/saved.cpp
    src/cards/valleyofshadows/shotgun.cpp
    src/cards/valleyofshadows/snake.cpp
    src/cards/valleyofshadows/tornado.cpp
    src/cards/valleyofshadows/tuco_franziskaner.cpp

    src/cards/wildwestshow/big_spencer.cpp
    src/cards/wildwestshow/bone_orchard.cpp
    src/cards/wildwestshow/darling_valentine.cpp
    src/cards/wildwestshow/gary_looter.cpp
    src/cards/wildwestshow/flint_westwood.cpp
    src/cards/wildwestshow/gary_looter.cpp
    src/cards/wildwestshow/greygory_deck.cpp
    src/cards/wildwestshow/helena_zontero.cpp
    src/cards/wildwestshow/john_pain.cpp
    src/cards/wildwestshow/ladyrosaoftexas.cpp
    src/cards/wildwestshow/miss_susanna.cpp
    src/cards/wildwestshow/ruleset.cpp
    src/cards/wildwestshow/sacagaway.cpp
    src/cards/wildwestshow/showdown.cpp
    src/cards/wildwestshow/teren_kill.cpp
    src/cards/wildwestshow/wildwestshow.cpp
    src/cards/wildwestshow/youl_grinner.cpp

    src/cards/armedanddangerous/add_cube.cpp
    src/cards/armedanddangerous/al_preacher.cpp
    src/cards/armedanddangerous/bandolier.cpp
    src/cards/armedanddangerous/bigfifty.cpp
    src/cards/armedanddangerous/bloody_mary.cpp
    src/cards/armedanddangerous/bomb.cpp
    src/cards/armedanddangerous/buntlinespecial.cpp
    src/cards/armedanddangerous/doublebarrel.cpp
    src/cards/armedanddangerous/duck.cpp
    src/cards/armedanddangerous/flintlock.cpp
    src/cards/armedanddangerous/frankie_canton.cpp
    src/cards/armedanddangerous/julie_cutter.cpp
    src/cards/armedanddangerous/ms_abigail.cpp
    src/cards/armedanddangerous/red_ringo.cpp
    src/cards/armedanddangerous/ruleset.cpp
    src/cards/armedanddangerous/rust.cpp
    src/cards/armedanddangerous/squaw.cpp
    src/cards/armedanddangerous/thunderer.cpp
    src/cards/armedanddangerous/tumbleweed.cpp

    src/cards/canyondiablo/bronco.cpp
    src/cards/canyondiablo/brothel.cpp
    src/cards/canyondiablo/card_sharper.cpp
    src/cards/canyondiablo/disarm.cpp
    src/cards/canyondiablo/graverobber.cpp
    src/cards/canyondiablo/indianguide.cpp
    src/cards/canyondiablo/lastwill.cpp
    src/cards/canyondiablo/mirage.cpp
    src/cards/canyondiablo/packmule.cpp
    src/cards/canyondiablo/ruleset.cpp
    src/cards/canyondiablo/sacrifice.cpp
    src/cards/canyondiablo/taxman.cpp

    src/cards/card_effect.cpp

    src/game/bot_ai.cpp
    src/game/card_serial.cpp
    src/game/discard_all.cpp
    src/game/durations.cpp
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
    src/game/request_queue.cpp

    src/net/chat_commands.cpp
    src/net/main.cpp
    src/net/manager.cpp
)

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

add_library(bangcommon INTERFACE)
target_include_directories(bangcommon INTERFACE src)
target_link_libraries(bangcommon INTERFACE cpputils networking)

add_executable(bangserver ${bangserver_srcs})
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