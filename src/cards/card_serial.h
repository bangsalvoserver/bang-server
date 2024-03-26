#ifndef __CARD_SERIAL_H__
#define __CARD_SERIAL_H__

#include "utils/small_pod.h"
#include "utils/utils.h"

namespace banggame {
    struct game_context;
    struct game;
    struct card;
    struct player;

    struct effect_vtable;
    struct equip_vtable;
    struct modifier_vtable;
    struct mth_vtable;

    namespace serial {

        using opt_card = banggame::card *;
        using card = not_null<opt_card>;
        using opt_player = banggame::player *;
        using player = not_null<opt_player>;
        using int_list = small_int_set;
        
        using player_list = std::vector<player>;
        using card_list = std::vector<card>;

        using effect_type = const banggame::effect_vtable *;
        using equip_type = const banggame::equip_vtable *;
        using modifier_type = const banggame::modifier_vtable *;
        using mth_type = const banggame::mth_vtable *;
        
        struct card_format {
            banggame::card *card;
            card_format() = default;
            card_format(banggame::card *card) : card(card) {}
        };

        template<typename T, typename ... Ts>
        static constexpr bool is_one_of = (std::is_same_v<T, Ts> || ...);

        template<typename T>
        concept serializable = is_one_of<T, opt_card, card, opt_player, player, card_format, effect_type, equip_type, modifier_type, mth_type>;

        template<typename T>
        concept deserializable = is_one_of<T, opt_card, card, opt_player, player>;
    }

}

namespace json {
    template<banggame::serial::serializable T> struct serializer<T, banggame::game_context> {
        const banggame::game_context &context;
        json operator()(same_if_trivial_t<T> value) const;
    };

    template<banggame::serial::deserializable T> struct deserializer<T, banggame::game_context> {
        const banggame::game_context &context;
        T operator()(const json &value) const;
    };

}

#endif