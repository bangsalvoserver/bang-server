#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "game/game_update.h"

#include "utils/json_serial.h"

namespace sdl {
    DEFINE_STRUCT(image_pixels,
        (int, width)
        (int, height)
        (std::vector<std::byte>, pixels)
    )
}

namespace banggame {

    DEFINE_STRUCT(user_info,
        (std::string, name)
        (sdl::image_pixels, profile_image)
    )

    DEFINE_STRUCT(connect_args,
        (user_info, user)
        (int, user_id)
        (std::string, commit_hash)
    )

    DEFINE_STRUCT(lobby_info,
        (std::string, name)
        (game_options, options)
    )

    DEFINE_STRUCT(lobby_id_args,
        (int, lobby_id)
    )

    DEFINE_STRUCT(lobby_chat_client_args,
        (std::string, message)
    )

    DEFINE_ENUM_TYPES(client_message_type,
        (connect, connect_args)
        (user_edit, user_info)
        (lobby_list)
        (lobby_make, lobby_info)
        (lobby_edit, lobby_info)
        (lobby_join, lobby_id_args)
        (lobby_leave)
        (lobby_chat, lobby_chat_client_args)
        (lobby_return)
        (game_start)
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
        (int, user_id)
    )

    DEFINE_STRUCT(lobby_data,
        (int, lobby_id)
        (std::string, name)
        (int, num_players)
        (lobby_state, state)
    )

    DEFINE_STRUCT(user_info_id_args,
        (int, user_id)
        (user_info, user)
    )

    DEFINE_STRUCT(user_id_args,
        (int, user_id)
    )

    DEFINE_STRUCT(lobby_chat_args,
        (int, user_id)
        (std::string, message)
    )

    DEFINE_ENUM_TYPES(server_message_type,
        (client_accepted, client_accepted_args)
        (lobby_error, std::string)
        (lobby_update, lobby_data)
        (lobby_entered, lobby_info)
        (lobby_edited, lobby_info)
        (lobby_removed, lobby_id_args)
        (lobby_owner, user_id_args)
        (lobby_add_user, user_info_id_args)
        (lobby_remove_user, user_id_args)
        (lobby_chat, lobby_chat_args)
        (game_update, json::json)
        (game_started)
    )

    using server_message = enums::enum_variant<server_message_type>;
    #define SRV_TAG(name) enums::enum_tag_t<banggame::server_message_type::name>

}

#endif