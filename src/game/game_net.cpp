#include "game/game_net.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "play_verify.h"

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
            } else {
                return nullptr;
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
            } else {
                return nullptr;
            }
        }
    };

    template<typename Context> struct serializer<banggame::card_format, Context> {
        json operator()(banggame::card_format value) const {
            if (value.card) {
                return {
                    {"name", value.card->name},
                    {"sign", serialize(value.card->sign)}
                };
            } else {
                return json::object();
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

}

namespace banggame {
    json::json game_net_manager::serialize_update(const game_update &update) const {
        return json::serialize(update, context());
    }
    
    ticks game_net_manager::get_total_update_time() const {
        return rn::max(context().players | rv::transform([&](const player &p) {
            return rn::accumulate(m_updates | rv::transform([&](const game_update_tuple &tup) {
                if (tup.duration >= ticks{0} && tup.target.matches(&p)) {
                    return tup.duration;
                }
                return ticks{0};
            }), ticks{0});
        }));
    }
    
    player *game_net_manager::find_player_by_userid(int user_id) const {
        for (player &p : context().players) {
            if (p.user_id == user_id) {
                return &p;
            }
        }
        return nullptr;
    }

    std::string game_net_manager::handle_game_action(int user_id, const json::json &value) {
        player *origin = find_player_by_userid(user_id);
        if (!origin) {
            return "ERROR_USER_NOT_CONTROLLING_PLAYER";
        }
        
        auto result = verify_and_play(origin, json::deserialize<game_action>(value, context()));

        enums::visit_indexed(overloaded{
            [&](enums::enum_tag_t<message_type::ok>) {
                origin->m_game->commit_updates();
            },
            [&](enums::enum_tag_t<message_type::error>, game_string message) {
                add_update<game_update_type::game_error>(update_target::includes_private(origin), std::move(message));
            },
            [&](enums::enum_tag_t<message_type::prompt>, game_string message) {
                add_update<game_update_type::game_prompt>(update_target::includes_private(origin), std::move(message));
            }
        }, result);
        return {};
    }
}