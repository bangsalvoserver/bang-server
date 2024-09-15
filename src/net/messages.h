#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "game/game_options.h"

#include "utils/image_pixels.h"
#include "utils/enum_bitset.h"
#include "utils/tagged_variant.h"

namespace banggame {
    
    using id_type = unsigned int;

    struct connect_args {
        std::string username;
        utils::image_pixels propic;
        id_type session_id;
    };

    enum class lobby_team {
        game_player,
        game_spectator,
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

    struct game_rejoin_args {
        int user_id;
    };

    using client_message = utils::tagged_variant<
        utils::tag<"pong">,
        utils::tag<"connect", connect_args>,
        utils::tag<"user_set_name", std::string>,
        utils::tag<"user_set_propic", utils::image_pixels>,
        utils::tag<"lobby_make", lobby_info>,
        utils::tag<"lobby_edit", lobby_info>,
        utils::tag<"lobby_join", lobby_id_args>,
        utils::tag<"lobby_leave">,
        utils::tag<"lobby_chat", lobby_chat_client_args>,
        utils::tag<"lobby_return">,
        utils::tag<"user_set_team", lobby_team>,
        utils::tag<"game_start">,
        utils::tag<"game_rejoin", game_rejoin_args>,
        utils::tag<"game_action", json::json>
    >;

    client_message deserialize_message(const json::json &value);

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

    enum class lobby_chat_flag {
        is_read
    };

    struct lobby_user_args {
        int user_id;
        std::string username;
        lobby_team team;
        enums::bitset<lobby_chat_flag> flags;
        std::chrono::milliseconds lifetime;
    };

    struct user_propic_args {
        int user_id;
        utils::image_pixels propic;
    };

    struct lobby_chat_args {
        int user_id;
        std::string username;
        std::string message;
        enums::bitset<lobby_chat_flag> flags;
    };

    using server_message = utils::tagged_variant<
        utils::tag<"ping">,
        utils::tag<"client_accepted", client_accepted_args>,
        utils::tag<"lobby_error", std::string>,
        utils::tag<"lobby_update", lobby_data>,
        utils::tag<"lobby_entered", lobby_entered_args>,
        utils::tag<"lobby_edited", lobby_info>,
        utils::tag<"lobby_removed", lobby_id_args>,
        utils::tag<"lobby_add_user", lobby_user_args>,
        utils::tag<"lobby_user_propic", user_propic_args>,
        utils::tag<"lobby_remove_user", int>,
        utils::tag<"lobby_kick">,
        utils::tag<"lobby_chat", lobby_chat_args>,
        utils::tag<"lobby_message", std::string>,
        utils::tag<"game_update", json::json>,
        utils::tag<"game_started">
    >;

    template<utils::fixed_string Name>
    concept server_message_type = utils::tag_for<utils::tag<Name>, server_message>;
    
    json::json serialize_message(const server_message &message);

}

#endif