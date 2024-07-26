#include "game/game_net.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "play_verify.h"
#include "game.h"

#include "utils/json_aggregate.h"

namespace json {

    template<typename Context> struct serializer<banggame::card_ptr, Context> {
        json operator()(banggame::card_ptr card) const {
            if (!card) {
                throw serialize_error("Cannot serialize card: value is null");
            }
            return card->id;
        }
    };

    template<> struct deserializer<banggame::card_ptr, banggame::game_context> {
        banggame::card_ptr operator()(missing_field) const {
            throw deserialize_error("Missing card field");
        }

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

    template<typename Context> struct serializer<banggame::player_ptr, Context> {
        json operator()(banggame::player_ptr player) const {
            if (!player) {
                throw serialize_error("Cannot serialize player: value is null");
            }
            return player->id;
        }
    };

    template<> struct deserializer<banggame::player_ptr, banggame::game_context> {
        banggame::player_ptr operator()(missing_field) const {
            throw deserialize_error("Missing player field");
        }

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
            return std::string(value->name);
        }
    };

    template<typename Context> struct serializer<const banggame::equip_vtable *, Context> {
        json operator()(const banggame::equip_vtable *value) const {
            return std::string(value->name);
        }
    };

    template<typename Context> struct serializer<const banggame::modifier_vtable *, Context> {
        json operator()(const banggame::modifier_vtable *value) const {
            if (value) {
                return std::string(value->name);
            } else {
                return {};
            }
        }
    };

    template<typename Context> struct serializer<const banggame::mth_vtable *, Context> {
        json operator()(const banggame::mth_vtable *value) const {
            if (value) {
                return std::string(value->name);
            } else {
                return {};
            }
        }
    };

    template<typename Context> struct serializer<banggame::format_arg_list, Context> {
        json operator()(const banggame::format_arg_list &list, const Context &ctx) const {
            auto ret = json::array();
            for (size_t i=0; i < list.size(); ++i) {
                auto [type, value] = list[i];
                switch (type) {
                case banggame::format_arg_list::format_number:
                    ret.push_back({{ "integer", value.number_value }});
                    break;
                case banggame::format_arg_list::format_card:
                    if (value.card_value) {
                        ret.push_back({{ "card", {
                            {"name", value.card_value->name},
                            {"sign", serialize_unchecked(value.card_value->sign, ctx)}
                        }}});
                    } else {
                        ret.push_back({{ "card", json::object() }});
                    }
                    break;
                case banggame::format_arg_list::format_player:
                    ret.push_back({{ "player", serialize_unchecked(utils::nullable{ value.player_value }, ctx) }});
                    break;
                }
            }
            return ret;
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
            }) | rn::to_vector, ctx);
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
            }) | rn::to_vector, ctx);
        };
    };

    template<typename Context> struct serializer<banggame::game_string, Context> {
        json operator()(const banggame::game_string &value, const Context &ctx) const {
            return {
                {"format_str", serialize_unchecked(value.format_str, ctx)},
                {"format_args", serialize_unchecked(value.format_args, ctx)}
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

        switch (result.type) {
        case message_type::ok:
            origin->m_game->commit_updates();
            break;
        case message_type::error:
            add_update<"game_error">(update_target::includes_private(origin), result.message);
            break;
        case message_type::prompt:
            add_update<"game_prompt">(update_target::includes_private(origin), result.message);
            break;
        }
    }
}