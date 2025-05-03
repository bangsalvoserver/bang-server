#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "game/game_options.h"

#include "utils/enum_bitset.h"

#include "image_pixels.h"

namespace banggame {
    
    using id_type = unsigned int;

    namespace client_messages {
        
        struct pong {};
        
        struct connect {
            std::string username;
            image_pixels propic;
            id_type session_id;
        };

        struct user_set_name {
            struct transparent{};
            std::string username;
        };

        struct user_set_propic {
            struct transparent{};
            image_pixels propic;
        };
        
        struct lobby_make {
            std::string name;
            std::string password;
            game_options options;
        };

        struct lobby_game_options {
            struct transparent{};
            game_options options;
        };

        struct lobby_join {
            id_type lobby_id;
            std::string password;
        };

        struct lobby_leave {};

        struct lobby_chat {
            std::string message;
        };

        struct lobby_return {};

        struct user_spectate {
            struct transparent{};
            bool spectator;
        };

        struct game_start {};

        struct game_rejoin {
            int user_id;
        };

        struct game_action {
            struct transparent{};
            json::json action;
        };
    }
    
    using client_message = std::variant<
        client_messages::pong,
        client_messages::connect,
        client_messages::user_set_name,
        client_messages::user_set_propic,
        client_messages::lobby_make,
        client_messages::lobby_game_options,
        client_messages::lobby_join,
        client_messages::lobby_leave,
        client_messages::lobby_chat,
        client_messages::lobby_return,
        client_messages::user_spectate,
        client_messages::game_start,
        client_messages::game_rejoin,
        client_messages::game_action
    >;

    client_message deserialize_message(std::string_view value);

    enum class lobby_state {
        waiting,
        playing,
        finished,
    };

    enum class game_user_flag {
        disconnected,
        lobby_owner,
        spectator,
        muted
    };

    using game_user_flags = enums::bitset<game_user_flag>;

    enum class lobby_chat_flag {
        is_read,
        translated,
    };

    using lobby_chat_flags = enums::bitset<lobby_chat_flag>;

    namespace chat_format_arg {
        struct user {
            struct transparent{};
            int value;
        };

        struct integer {
            struct transparent{};
            int value;
        };

        struct string {
            struct transparent{};
            std::string value;
        };
    }

    namespace server_messages {
        
        struct ping {};

        struct client_accepted {
            id_type session_id;
        };

        struct lobby_error {
            struct transparent{};
            std::string message;
        };

        struct lobby_update {
            id_type lobby_id;
            std::string name;
            int num_players;
            int num_bots;
            int num_spectators;
            int max_players;
            bool secure;
            lobby_state state;
        };

        struct lobby_entered {
            int user_id;
            id_type lobby_id;
            std::string name;
            game_options options;
        };

        struct lobby_game_options {
            struct transparent{};
            game_options options;
        };

        struct lobby_removed {
            id_type lobby_id;
        };

        struct lobby_user_update {
            int user_id;
            std::string username;
            image_pixels_hash propic;
            game_user_flags flags;
            std::chrono::milliseconds lifetime;
        };

        struct lobby_kick {};

        using chat_format_args = std::vector<std::variant<
            chat_format_arg::user,
            chat_format_arg::integer,
            chat_format_arg::string
        >>;

        struct lobby_chat {
            int user_id;
            std::string message;
            chat_format_args args;
            lobby_chat_flags flags;
        };

        struct game_update {
            struct transparent{};
            json::json update;
        };

        struct game_started {};
    }

    using server_message = std::variant<
        server_messages::ping,
        server_messages::client_accepted,
        server_messages::lobby_error,
        server_messages::lobby_update,
        server_messages::lobby_entered,
        server_messages::lobby_game_options,
        server_messages::lobby_removed,
        server_messages::lobby_user_update,
        server_messages::lobby_kick,
        server_messages::lobby_chat,
        server_messages::game_update,
        server_messages::game_started
    >;
    
    std::string serialize_message(const server_message &message);

}

#endif