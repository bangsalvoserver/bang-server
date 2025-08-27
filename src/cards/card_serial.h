#ifndef __CARD_SERIAL_H__
#define __CARD_SERIAL_H__

#include "game/game_table.h"

#include "utils/json_aggregate.h"
#include "utils/tagged_variant.h"

namespace json {

    template<typename T, typename U>
    concept maybe_const = std::is_same_v<
        std::remove_const_t<std::remove_pointer_t<T>>,
        std::remove_pointer_t<U>
    >;

    template<maybe_const<banggame::card_ptr> Card, typename Context>
    struct serializer<Card, Context> {
        static void write(Card card, string_writer &writer) {
            if (!card) {
                throw serialize_error("Cannot serialize card: value is null");
            }
            writer.Int(card->id);
        }
    };

    template<> struct deserializer<banggame::card_ptr, banggame::game_context> {
        static banggame::card_ptr read(const json &value, const banggame::game_context &context) {
            if (!value.IsInt()) {
                throw deserialize_error("Cannot deserialize card: value is not an integer");
            }
            int card_id = value.GetInt();
            if (banggame::card_ptr card = context.find_card(card_id)) {
                return card;
            }
            throw deserialize_error(std::format("Cannot find card {}", card_id));
        }
    };

    template<maybe_const<banggame::player_ptr> Player, typename Context>
    struct serializer<Player, Context> {
        static void write(Player player, string_writer &writer) {
            if (!player) {
                throw serialize_error("Cannot serialize player: value is null");
            }
            writer.Int(player->id);
        }
    };

    template<> struct deserializer<banggame::player_ptr, banggame::game_context> {
        static banggame::player_ptr read(const json &value, const banggame::game_context &context) {
            if (!value.IsInt()) {
                throw deserialize_error("Cannot deserialize player: value is not an integer");
            }
            int player_id = value.GetInt();
            if (banggame::player_ptr player = context.find_player(player_id)) {
                return player;
            }
            throw deserialize_error(std::format("Cannot find player {}", player_id));
        }
    };

}

#endif