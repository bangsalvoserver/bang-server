#ifndef __CARD_SERIAL_H__
#define __CARD_SERIAL_H__

#include "utils/json_serial.h"

namespace banggame {
    struct game;
    struct card;
    struct player;
}

template<typename T, typename ... Ts>
static constexpr bool is_one_of = (std::is_same_v<T, Ts> || ...);

#ifdef BUILD_BANG_CLIENT

namespace banggame {
    
    class card_view;
    class player_view;
    class game_scene;
    struct cube_widget;

    using player_card_pair = std::pair<player_view *, card_view *>;
    using card_cube_pair = std::pair<card_view *, cube_widget *>;

    namespace serial {
        using context = banggame::game_scene;
        using card = banggame::card_view *;
        using player = banggame::player_view *;
        using cube = banggame::card_cube_pair;
        using player_card = banggame::player_card_pair;

        template<typename T>
        concept serializable = is_one_of<T, card, player, cube, player_card>;
    }
}

#else

namespace banggame::serial {
    using context = banggame::game;
    using card = banggame::card *;
    using player = banggame::player *;
    using cube = banggame::card *;
    using player_card = banggame::card *;

    template<typename T>
    concept serializable = is_one_of<T, card, player>;
}

#ifndef BUILD_BANG_SERVER
    #define NO_DEFINE_SERIALIZERS
#endif

#endif

#ifndef NO_DEFINE_SERIALIZERS

namespace json {
    template<banggame::serial::serializable T> struct serializer<T, banggame::serial::context> {
        const banggame::serial::context &context;
        Json::Value operator()(T value) const;
    };

    template<banggame::serial::serializable T> struct deserializer<T, banggame::serial::context> {
        const banggame::serial::context &context;
        T operator()(const Json::Value &value) const;
    };

}

#endif

#endif