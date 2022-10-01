#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "game_update.h"
#include "json/value.h"

namespace sdl {
    struct image_pixels {REFLECTABLE(
        (int) width,
        (int) height,
        (std::vector<std::byte>) pixels
    )};
}

namespace banggame {

    struct connect_args {REFLECTABLE(
        (std::string) user_name,
        (sdl::image_pixels) profile_image,
        (std::string) commit_hash
    )};

    struct lobby_info {REFLECTABLE(
        (std::string) name,
        (game_options) options
    )};

    struct lobby_id_args {REFLECTABLE(
        (int) lobby_id
    )};

    struct lobby_rejoin_args {REFLECTABLE(
        (int) player_id
    )};

    struct lobby_chat_client_args {REFLECTABLE(
        (std::string) message
    )};

    DEFINE_ENUM_TYPES(client_message_type,
        (connect, connect_args)
        (lobby_list)
        (lobby_make, lobby_info)
        (lobby_edit, lobby_info)
        (lobby_join, lobby_id_args)
        (lobby_rejoin, lobby_rejoin_args)
        (lobby_leave)
        (lobby_chat, lobby_chat_client_args)
        (lobby_return)
        (game_start)
        (game_action, Json::Value)
    )

    using client_message = enums::enum_variant<client_message_type>;
    #define MSG_TAG(name) enums::enum_tag_t<banggame::client_message_type::name>

    DEFINE_ENUM(lobby_state,
        (waiting)
        (playing)
        (finished)
    )

    struct client_accepted_args {REFLECTABLE(
        (int) user_id
    )};

    struct lobby_data {REFLECTABLE(
        (int) lobby_id,
        (std::string) name,
        (int) num_players,
        (lobby_state) state
    )};

    struct lobby_add_user_args {REFLECTABLE(
        (int) user_id,
        (std::string) name,
        (sdl::image_pixels) profile_image
    )};

    struct user_id_args {REFLECTABLE(
        (int) user_id
    )};

    struct lobby_chat_args {REFLECTABLE(
        (int) user_id,
        (std::string) message
    )};

    DEFINE_ENUM_TYPES(server_message_type,
        (client_accepted, client_accepted_args)
        (lobby_error, std::string)
        (lobby_update, lobby_data)
        (lobby_entered, lobby_info)
        (lobby_edited, lobby_info)
        (lobby_removed, lobby_id_args)
        (lobby_owner, user_id_args)
        (lobby_add_user, lobby_add_user_args)
        (lobby_remove_user, user_id_args)
        (lobby_chat, lobby_chat_args)
        (game_update, Json::Value)
        (game_started)
    )

    using server_message = enums::enum_variant<server_message_type>;
    #define SRV_TAG(name) enums::enum_tag_t<banggame::server_message_type::name>

}

#endif