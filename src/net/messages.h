#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "game/game_update.h"

#include "utils/json_serial.h"

namespace sdl {
    DEFINE_STRUCT(image_pixels,
        (int, width)
        (int, height)
        (base64::encoded_bytes, pixels)
    )
}

namespace banggame {
    
    using id_type = unsigned int;

    DEFINE_STRUCT(user_info,
        (std::string, name)
        (sdl::image_pixels, profile_image)
    )

    DEFINE_STRUCT(connect_args,
        (user_info, user)
        (id_type, session_id)
        (std::string, commit_hash)
    )

    DEFINE_STRUCT(lobby_info,
        (std::string, name)
        (game_options, options)
    )

    DEFINE_STRUCT(lobby_id_args,
        (id_type, lobby_id)
    )

    DEFINE_STRUCT(lobby_chat_client_args,
        (std::string, message)
    )

    DEFINE_ENUM_TYPES(client_message_type,
        (pong)
        (connect, connect_args)
        (user_edit, user_info)
        (lobby_make, lobby_info)
        (lobby_edit, lobby_info)
        (lobby_join, lobby_id_args)
        (lobby_leave)
        (lobby_chat, lobby_chat_client_args)
        (lobby_return)
        (game_start)
        (game_rejoin, int)
        (game_action, json::json)
    )

    using client_message = enums::enum_variant<client_message_type>;
    #define MSG_TAG(name) enums::enum_tag_t<banggame::client_message_type::name>

    DEFINE_ENUM(lobby_state,
        (waiting)
        (playing)
        (finished)
    )

    DEFINE_STRUCT(client_accepted_args,
        (id_type, session_id)
    )

    DEFINE_STRUCT(lobby_data,
        (id_type, lobby_id)
        (std::string, name)
        (int, num_players)
        (int, num_spectators)
        (int, max_players)
        (lobby_state, state)
    )

    DEFINE_STRUCT(lobby_entered_args,
        (int, user_id)
        (id_type, lobby_id)
        (std::string, name)
        (game_options, options)
    )

    DEFINE_STRUCT(user_info_id_args,
        (int, user_id)
        (user_info, user)
        (bool, is_read)
        (std::chrono::milliseconds, lifetime)
    )

    DEFINE_STRUCT(lobby_chat_args,
        (int, user_id)
        (std::string, message)
        (bool, is_read)
    )

    DEFINE_ENUM_TYPES(server_message_type,
        (ping)
        (client_accepted, client_accepted_args)
        (client_count, int)
        (lobby_error, std::string)
        (lobby_update, lobby_data)
        (lobby_entered, lobby_entered_args)
        (lobby_edited, lobby_info)
        (lobby_removed, lobby_id_args)
        (lobby_add_user, user_info_id_args)
        (lobby_remove_user, int)
        (lobby_kick)
        (lobby_chat, lobby_chat_args)
        (game_update, json::json)
        (game_started)
    )

    using server_message = enums::enum_variant<server_message_type>;
    #define SRV_TAG(name) enums::enum_tag_t<banggame::server_message_type::name>

}

#endif