#include "game_net.h"

#include "cards/card_serial.h"
#include "cards/expansion_set.h"
#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "request_timer.h"
#include "play_verify.h"

namespace json {
    
    template<typename Context> struct serializer<banggame::tag_map, Context> {
        json operator()(const banggame::tag_map &map) const {
            auto result = json::object();
            for (const auto &[k, v] : map) {
                result.push_back({enums::to_string(k), v});
            }
            return result;
        }
    };

    template<> struct serializer<const void *, banggame::game_context> {
        struct skip_field{};
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

    template<typename Context> struct serializer<const banggame::targeting_vtable *, Context> {
        json operator()(const banggame::targeting_vtable *value) const {
            return value->name;
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

    static card_targets_pair deserialize_card_targets(const json::json &card, const json::json &targets, const game_context &context, bool check_equip = false) {
        card_targets_pair result {
            .card = json::deserialize<card_ptr, game_context>(card, context)
        };

        if (check_equip && result.card->is_equip_card()) {
            if (result.card->self_equippable()) {
                if (!targets.empty()) {
                    throw json::deserialize_error("Self equippable card must have no targets");
                }
            } else {
                if (targets.size() != 1) {
                    throw json::deserialize_error("Equip card must have one target");
                }
                result.targets.emplace_back(json::deserialize<player_ptr, game_context>(targets[0], context));
            }
        } else {
            bool is_response = result.card->m_game->pending_requests();

            const auto &effects = result.card->get_effect_list(is_response);

            if (effects.empty()) {
                throw json::deserialize_error("Effect list is empty");
            }

            if (effects.size() != targets.size()) {
                throw json::deserialize_error("Invalid number of targets");
            }
            
            result.targets.reserve(effects.size());
            for (const auto &[effect, target] : rv::zip(effects, targets)) {
                result.targets.push_back(effect.target->deserialize_json(target, context));
            }
        }

        return result;
    }

    static const json::json &get_value(const json::json &obj, std::string_view key) {
        auto it = obj.find(key);
        if (it == obj.end()) {
            throw json::deserialize_error(std::format("Missing key {} in object", key));
        }
        return *it;
    }

    static modifier_list deserialize_modifier_list(const json::json &value, const game_context &context) {
        if (!value.is_array()) {
            throw json::deserialize_error("Cannot deserialize modifier_list: value is not an array");
        }
        modifier_list result;
        result.reserve(value.size());
        for (const json::json &modifier : value) {
            if (!modifier.is_object()) {
                throw json::deserialize_error("Cannot deserialize modifier_pair: value is not an object");
            }
            result.push_back(deserialize_card_targets(get_value(modifier, "card"), get_value(modifier, "targets"), context));
        }
        return result;
    }

    static game_action deserialize_game_action(const json::json &value, const game_context &context) {
        auto [card, targets] = deserialize_card_targets(get_value(value, "card"), get_value(value, "targets"), context, true);
        return {
            .card = card,
            .modifiers = deserialize_modifier_list(get_value(value, "modifiers"), context),
            .targets = std::move(targets),
            .bypass_prompt = json::deserialize<bool>(get_value(value, "bypass_prompt"))
        };
    }

    json::json game_net_manager::serialize_update(const game_update &update) const {
        return json::serialize<game_update, game_context>(update, *this);
    }

    void game_net_manager::handle_game_action(player_ptr origin, const json::json &value) {
        if (!value.is_object()) {
            throw json::deserialize_error("Cannot deserialize game_action: value is not an object");
        }
        
        std::optional<timer_id_t> timer_id, current_timer_id;
        if (auto it = value.find("timer_id"); it != value.end()) {
            timer_id = json::deserialize<timer_id_t>(*it);
        }
        if (auto timer = origin->m_game->top_request<request_timer>(); timer && timer->enabled()) {
            current_timer_id = timer->get_timer_id();
        }

        if (timer_id != current_timer_id) {
            add_update(update_target::includes(origin), game_updates::game_error{ "ERROR_TIMER_EXPIRED" });
            return;
        }

        auto action = deserialize_game_action(value, *this);
        auto result = verify_and_play(origin, action);

        std::visit(overloaded{
            [&](play_verify_results::ok) {
                origin->m_game->commit_updates();
            },
            [&](play_verify_results::error error) {
                add_update(update_target::includes(origin), game_updates::game_error{ error.message });
            },
            [&](play_verify_results::prompt prompt) {
                add_update(update_target::includes(origin), game_updates::game_prompt{ prompt.message.message });
            }
        }, result);
    }
}