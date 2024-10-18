#include "game/game_net.h"

#include "cards/expansion_set.h"
#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "play_verify.h"
#include "game.h"

#include "utils/json_aggregate.h"

namespace json {

    template<typename T, typename U>
    concept maybe_const = std::is_same_v<
        std::remove_const_t<std::remove_pointer_t<T>>,
        std::remove_pointer_t<U>
    >;

    template<maybe_const<banggame::card_ptr> Card, typename Context>
    struct serializer<Card, Context> {
        json operator()(Card card) const {
            if (!card) {
                throw serialize_error("Cannot serialize card: value is null");
            }
            return card->id;
        }
    };

    template<> struct deserializer<banggame::card_ptr, banggame::game_context> {
        banggame::card_ptr operator()(const json &value, const banggame::game_context &context) const {
            if (!value.is_number_integer()) {
                throw deserialize_error("Cannot deserialize card: value is not an integer");
            }
            int card_id = value.get<int>();
            if (banggame::card_ptr card = context.find_card(card_id)) {
                return card;
            }
            throw deserialize_error(std::format("Cannot find card {}", card_id));
        }
    };

    template<maybe_const<banggame::player_ptr> Player, typename Context>
    struct serializer<Player, Context> {
        json operator()(Player player) const {
            if (!player) {
                throw serialize_error("Cannot serialize player: value is null");
            }
            return player->id;
        }
    };

    template<> struct deserializer<banggame::player_ptr, banggame::game_context> {
        banggame::player_ptr operator()(const json &value, const banggame::game_context &context) const {
            if (!value.is_number_integer()) {
                throw deserialize_error("Cannot deserialize player: value is not an integer");
            }
            int player_id = value.get<int>();
            if (banggame::player_ptr player = context.find_player(player_id)) {
                return player;
            }
            throw deserialize_error(std::format("Cannot find player {}", player_id));
        }
    };

    template<typename Context> struct serializer<banggame::tag_map, Context> {
        json operator()(const banggame::tag_map &map) const {
            auto result = json::object();
            for (const auto &[k, v] : map) {
                result.push_back({enums::to_string(k), v});
            }
            return result;
        }
    };

    template<typename Context> struct serializer<const banggame::effect_vtable *, Context> {
        json operator()(const banggame::effect_vtable *value) const {
            return value->name;
        }
    };

    template<typename Context> struct serializer<const banggame::equip_vtable *, Context> {
        json operator()(const banggame::equip_vtable *value) const {
            return value->name;
        }
    };

    template<typename Context> struct serializer<const banggame::modifier_vtable *, Context> {
        json operator()(const banggame::modifier_vtable *value) const {
            if (value) {
                return value->name;
            } else {
                return {};
            }
        }
    };

    template<typename Context> struct serializer<const banggame::mth_vtable *, Context> {
        json operator()(const banggame::mth_vtable *value) const {
            if (value) {
                return value->name;
            } else {
                return {};
            }
        }
    };

    template<typename Context> struct serializer<banggame::card_backface_list, Context> {
        struct card_backface {
            int id;
            banggame::card_deck_type deck;
        };

        json operator()(const banggame::card_backface_list &value, const Context &ctx) const {
            return serialize_unchecked(value.cards | rv::transform([](banggame::const_card_ptr card) {
                return card_backface{ card->id, card->deck };
            }), ctx);
        }
    };

    template<typename Context> struct serializer<banggame::player_user_list, Context> {
        struct player_user_pair {
            int player_id;
            int user_id;
        };

        json operator()(const banggame::player_user_list &value, const Context &ctx) const {
            return serialize_unchecked(value.players | rv::transform([](banggame::const_player_ptr player) {
                return player_user_pair{ player->id, player->user_id };
            }), ctx);
        };
    };

    struct game_string_tag {};

    template<> struct serializer<utils::nullable<banggame::const_card_ptr>, game_string_tag> {
        struct format_card {
            std::string_view name;
            banggame::card_sign sign;
        };

        json operator()(utils::nullable<banggame::const_card_ptr> value, const game_string_tag &ctx) const {
            if (value) {
                return serialize_unchecked(format_card{ value->name, value->sign }, ctx);
            } else {
                return json::object();
            }
        }
    };

    template<typename Context> struct serializer<banggame::game_string, Context> {
        json operator()(const banggame::game_string &value) const {
            return {
                {"format_str", std::string_view(value.format_str)},
                {"format_args", serialize_unchecked(value.format_args, game_string_tag{})}
            };
        }
    };

    template<> struct serializer<banggame::animation_duration, banggame::game_context> {
        json operator()(const banggame::animation_duration &duration, const banggame::game_context &context) const {
            return serialize_unchecked(context.transform_duration(duration.get()), context);
        }
    };

}

namespace banggame {
    json::json game_net_manager::serialize_update(const game_update &update) const {
        return json::serialize<game_update, game_context>(update, *this);
    }

    void game_net_manager::handle_game_action(player_ptr origin, const json::json &value) {
        auto action = json::deserialize<game_action, game_context>(value, *this);
        auto result = verify_and_play(origin, action);

        utils::visit_tagged(overloaded{
            [&](utils::tag<"ok">) {
                origin->m_game->commit_updates();
            },
            [&](utils::tag<"error">, game_string error) {
                add_update<"game_error">(update_target::includes_private(origin), error);
            },
            [&](utils::tag<"prompt">, prompt_string prompt) {
                add_update<"game_prompt">(update_target::includes_private(origin), prompt.message);
            }
        }, result);
    }
}