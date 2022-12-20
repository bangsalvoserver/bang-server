add_subdirectory(target_types)

target_sources(bangserver
PRIVATE
    bot_ai.cpp
    bot_suggestion.cpp
    game.cpp
    game_table.cpp
    draw_check_handler.cpp
    durations.cpp
    player.cpp
    player_iterator.cpp
    play_verify.cpp
    possible_to_play.cpp
    request_queue.cpp
)