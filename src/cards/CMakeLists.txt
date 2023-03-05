add_subdirectory(armedanddangerous)
add_subdirectory(base)
add_subdirectory(canyondiablo)
add_subdirectory(dodgecity)
add_subdirectory(fistfulofcards)
add_subdirectory(goldrush)
add_subdirectory(greattrainrobbery)
add_subdirectory(highnoon)
add_subdirectory(valleyofshadows)
add_subdirectory(wildwestshow)

target_sources(bangserver PRIVATE
    card_effect.cpp
    card_serial.cpp
    filters.cpp
    holders.cpp
)