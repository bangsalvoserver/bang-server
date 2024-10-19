include(FindPython3)
find_package(Python3 COMPONENTS Interpreter)
 
set(bang_cards_cpp "${CMAKE_CURRENT_BINARY_DIR}/bang_cards.cpp")
add_custom_command(
    OUTPUT "${bang_cards_cpp}"
    COMMAND ${Python3_EXECUTABLE} parse_bang_cards.py
        bang_cards.yml
        "${bang_cards_cpp}"
    VERBATIM
    DEPENDS
        bang_cards.yml
        sets/base.yml
        sets/dodgecity.yml
        sets/valleyofshadows.yml
        sets/udolistinu.yml
        sets/goldrush.yml
        sets/armedanddangerous.yml
        sets/greattrainrobbery.yml
        sets/highnoon.yml
        sets/fistfulofcards.yml
        sets/wildwestshow.yml
        sets/thebullet.yml
        sets/canyondiablo.yml
        parse_bang_cards.py
        cpp_generator.py
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
add_library(bang_cards_obj OBJECT "${bang_cards_cpp}")
target_include_directories(bang_cards_obj PRIVATE ..)
target_link_libraries(bang_cards_obj PRIVATE banglibs)

target_link_libraries(bangserver PRIVATE bang_cards_obj)

set(bot_info_cpp "${CMAKE_CURRENT_BINARY_DIR}/bot_info.cpp")
add_custom_command(
    OUTPUT "${bot_info_cpp}"
    COMMAND ${Python3_EXECUTABLE} parse_bots.py
        bot_info.yml
        "${bot_info_cpp}"
    VERBATIM
    DEPENDS
        bot_info.yml
        bot_propics/bot_icon_1.png
        bot_propics/bot_icon_2.png
        bot_propics/bot_icon_3.png
        bot_propics/bot_icon_4.png
        bot_propics/bot_icon_5.png
        bot_propics/bot_icon_6.png
        bot_propics/bot_icon_7.png
        bot_propics/bot_icon_8.png
        bot_propics/bot_icon_9.png
        bot_propics/bot_icon_10.png
        bot_propics/bot_icon_11.png
        bot_propics/bot_icon_12.png
        parse_bots.py
        cpp_generator.py
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
add_library(bot_info_obj OBJECT "${bot_info_cpp}")
target_include_directories(bot_info_obj PRIVATE ..)
target_link_libraries(bot_info_obj PRIVATE banglibs)

target_link_libraries(bangserver PRIVATE bot_info_obj)