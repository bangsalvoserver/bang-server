#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "game/game_update.h"

#include "utils/json_serial.h"

namespace sdl {
    struct image_pixels {
        int width;
        int height;
        base64::encoded_bytes pixels;
    };
}

namespace banggame {
    
    using id_type = unsigned int;

    struct user_info {
        std::string name;
        sdl::image_pixels profile_image;
    };

    struct connect_args {
        user_info user;
        id_type session_id;
        std::string commit_hash;
    };

    struct lobby_info {
        std::string name;
        game_options options;
    };

    struct lobby_id_args {
        id_type lobby_id;
    };

    struct lobby_chat_client_args {
        std::string message;
    };

    enum class client_message_type {
        pong,
        connect,
        user_edit,
        lobby_make,
        lobby_edit,
        lobby_join,
        lobby_leave,
        lobby_chat,
        lobby_return,
        game_start,
        game_rejoin,
        game_action,
    };

    using client_message = enums::enum_variant<client_message_type,
        enums::type_assoc<client_message_type::connect, connect_args>,
        enums::type_assoc<client_message_type::user_edit, user_info>,
        enums::type_assoc<client_message_type::lobby_make, lobby_info>,
        enums::type_assoc<client_message_type::lobby_edit, lobby_info>,
        enums::type_assoc<client_message_type::lobby_join, lobby_id_args>,
        enums::type_assoc<client_message_type::lobby_chat, lobby_chat_client_args>,
        enums::type_assoc<client_message_type::game_rejoin, int>,
        enums::type_assoc<client_message_type::game_action, json::json>
    >;

    #define MSG_TAG(name) enums::enum_tag_t<banggame::client_message_type::name>

    enum class lobby_state {
        waiting,
        playing,
        finished,
    };

    struct client_accepted_args {
        id_type session_id;
    };

    struct lobby_data {
        id_type lobby_id;
        std::string name;
        int num_players;
        int num_spectators;
        int max_players;
        lobby_state state;
    };

    struct lobby_entered_args {
        int user_id;
        id_type lobby_id;
        std::string name;
        game_options options;
    };

    struct user_info_id_args {
        int user_id;
        user_info user;
        bool is_read;
        std::chrono::milliseconds lifetime;
    };

    struct lobby_chat_args {
        int user_id;
        std::string message;
        bool is_read;
    };

    enum class server_message_type {
        ping,
        client_accepted,
        client_count,
        lobby_error,
        lobby_update,
        lobby_entered,
        lobby_edited,
        lobby_removed,
        lobby_add_user,
        lobby_remove_user,
        lobby_kick,
        lobby_chat,
        game_update,
        game_started,
    };

    using server_message = enums::enum_variant<server_message_type,
        enums::type_assoc<server_message_type::client_accepted, client_accepted_args>,
        enums::type_assoc<server_message_type::client_count, int>,
        enums::type_assoc<server_message_type::lobby_error, std::string>,
        enums::type_assoc<server_message_type::lobby_update, lobby_data>,
        enums::type_assoc<server_message_type::lobby_entered, lobby_entered_args>,
        enums::type_assoc<server_message_type::lobby_edited, lobby_info>,
        enums::type_assoc<server_message_type::lobby_removed, lobby_id_args>,
        enums::type_assoc<server_message_type::lobby_add_user, user_info_id_args>,
        enums::type_assoc<server_message_type::lobby_remove_user, int>,
        enums::type_assoc<server_message_type::lobby_chat, lobby_chat_args>,
        enums::type_assoc<server_message_type::game_update, json::json>
    >;

}

#endif