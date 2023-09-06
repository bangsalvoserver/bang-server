set(bang_cards_cpp "${CMAKE_CURRENT_BINARY_DIR}/bang_cards.cpp")
add_custom_command(
    OUTPUT "${bang_cards_cpp}"
    COMMAND python3 parse_bang_cards.py
        bang_cards.yml
        "${bang_cards_cpp}"
    VERBATIM
    DEPENDS
        bang_cards.yml
        sets/base.yml
        sets/dodgecity.yml
        sets/valleyofshadows.yml
        sets/goldrush.yml
        sets/armedanddangerous.yml
        sets/greattrainrobbery.yml
        sets/highnoon.yml
        sets/fistfulofcards.yml
        sets/wildwestshow.yml
        sets/thebullet.yml
        sets/canyondiablo.yml
        parse_bang_cards.py
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
add_library(bang_cards_obj OBJECT "${bang_cards_cpp}")
target_include_directories(bang_cards_obj PRIVATE ..)
target_link_libraries(bang_cards_obj PRIVATE cpputils)
if(MINGW)
    # disable optimization, fix ICE in gcc 13 in mingw
    target_compile_options(bang_cards_obj PRIVATE -O0)
endif()

target_link_libraries(bangserver PRIVATE bang_cards_obj)