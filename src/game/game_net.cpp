#include "game/game_net.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "play_verify.h"

#include "utils/json_aggregate.h"

namespace json {

    template<std::convertible_to<banggame::card *> Card, typename Context> struct serializer<Card, Context> {
        json operator()(banggame::card *card) const {
            if (card) {
                return card->id;
            } else {
                return {};
            }
        }
    };

    template<std::convertible_to<banggame::card *> Card> struct deserializer<Card, banggame::game_context> {
        const banggame::game_context &context;
        banggame::card *operator()(const json &value) const {
            if (value.is_number_integer()) {
                return context.find_card(value.get<int>());
            } else if (value.is_null()) {
                return nullptr;
            } else {
                throw std::runtime_error("Cannot deserialize card: value is not an integer");
            }
        }
    };

    template<std::convertible_to<banggame::player *> Player, typename Context> struct serializer<Player, Context> {
        json operator()(banggame::player *player) const {
            if (player) {
                return player->id;
            } else {
                return {};
            }
        }
    };

    template<std::convertible_to<banggame::player *> Player> struct deserializer<Player, banggame::game_context> {
        const banggame::game_context &context;
        banggame::player *operator()(const json &value) const {
            if (value.is_number_integer()) {
                return context.find_player(value.get<int>());
            } else if (value.is_null()) {
                return nullptr;
            } else {
                throw std::runtime_error("Cannot deserialize player: value is not an integer");
            }
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
        json operator()(const banggame::format_arg_list &list) const {
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
                            {"sign", serialize(value.card_value->sign)}
                        }}});
                    } else {
                        ret.push_back({{ "card", json::object() }});
                    }
                    break;
                case banggame::format_arg_list::format_player:
                    ret.push_back({{ "player", serialize(value.player_value) }});
                    break;
                }
            }
            return ret;
        }
    };

    template<typename Context> struct serializer<banggame::game_string, Context> {
        json operator()(const banggame::game_string &value) const {
            return {
                {"format_str", serialize(value.format_str)},
                {"format_args", serialize(value.format_args)}
            };
        }
    };

    template<> struct serializer<banggame::animation_duration, banggame::game_context> {
        const banggame::game_context &context;
        json operator()(const banggame::animation_duration &duration) const {
            return serialize(context.transform_duration(duration.get()));
        }
    };

}

namespace banggame {
    json::json game_net_manager::serialize_update(const game_update &update) const {
        return json::serialize<game_update, game_context>(update, *this);
    }

    std::string game_net_manager::handle_game_action(int user_id, const json::json &value) {
        player *origin = find_player_by_userid(user_id);
        if (!origin) {
            return "ERROR_USER_NOT_CONTROLLING_PLAYER";
        }
        
        auto result = verify_and_play(origin, json::deserialize<game_action, game_context>(value, *this));

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
        
        return {};
    }
}