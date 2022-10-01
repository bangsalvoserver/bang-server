#ifndef __CARD_SERIAL_H__
#define __CARD_SERIAL_H__

#include "utils/json_serial.h"

namespace banggame {
    struct game;
    struct card;
    struct player;
}

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
    }
}

#else

namespace banggame::serial {
    using context = banggame::game;
    using card = banggame::card *;
    using player = banggame::player *;
    using cube = banggame::card *;
    using player_card = banggame::card *;
}

#ifndef BUILD_BANG_SERVER
    #define NO_DEFINE_SERIALIZERS
#endif

#endif

#ifndef NO_DEFINE_SERIALIZERS

namespace json {
    namespace serial = banggame::serial;

    template<> struct serializer<serial::card, serial::context> {
        const serial::context &context;
        Json::Value operator()(serial::card card) const;
    };

    template<> struct serializer<serial::player, serial::context> {
        const serial::context &context;
        Json::Value operator()(serial::player player) const;
    };

    template<> struct deserializer<serial::card, serial::context> {
        const serial::context &context;
        serial::card operator()(const Json::Value &value) const;
    };

    template<> struct deserializer<serial::player, serial::context> {
        const serial::context &context;
        serial::player operator()(const Json::Value &value) const;
    };

#ifdef BUILD_BANG_CLIENT

    template<> struct serializer<serial::cube, serial::context> {
        const serial::context &context;
        Json::Value operator()(serial::cube cube) const;
    };

    template<> struct serializer<serial::player_card, serial::context> {
        const serial::context &context;
        Json::Value operator()(serial::player_card pair) const;
    };

    template<> struct deserializer<serial::cube, serial::context> {
        const serial::context &context;
        serial::cube operator()(const Json::Value &value) const;
    };

    template<> struct deserializer<serial::player_card, serial::context> {
        const serial::context &context;
        serial::player_card operator()(const Json::Value &value) const;
    };

#endif

}

#endif

#endif