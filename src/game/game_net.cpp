#include "game_net.h"

#include "cards/card_serial.h"
#include "cards/expansion_set.h"
#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "request_timer.h"
#include "play_verify.h"

#include "net/lobby.h"

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

    template<typename Context> struct serializer<const banggame::effect_vtable *, Context> {
        json operator()(const banggame::effect_vtable *value) const {
            return value->name;
        }
    };

    template<typename Context> struct serializer<const banggame::targeting_vtable *, Context> {
        json operator()(const banggame::targeting_vtable *value) const {
            return value->name;
        }
    };

    template<> struct serializer<const void *, banggame::game_context> {
        struct skip_field{};
    };

    template<> struct serializer<banggame::effect_holder, banggame::game_context> : aggregate_serializer_unchecked<banggame::effect_holder, banggame::game_context> {
        json operator()(const banggame::effect_holder &effect, const banggame::game_context &ctx) const {
            json result = aggregate_serializer_unchecked<banggame::effect_holder, banggame::game_context>::operator()(effect, ctx);
            json value = effect.target->serialize_args(effect, ctx);
            if (value.is_object()) {
                for (auto &[key, val] : value.items()) {
                    result[key] = std::move(val);
                }
            }
            return result;
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

    template<> struct serializer<banggame::format_arg_value, banggame::game_context> {
        json serialize_card(int card_id, const banggame::game_context &ctx) const {
            struct format_card {
                std::string_view name;
                banggame::card_sign sign;
            };

            if (card_id != 0) {
                banggame::card_ptr target_card = ctx.find_card(card_id);
                return serialize_unchecked(format_card{ target_card->name, target_card->sign }, ctx);
            } else {
                return json::object();
            }
        }

        json serialize_player(int player_id, const banggame::game_context &ctx) const {
            if (player_id != 0) {
                banggame::player_ptr target = ctx.find_player(player_id);
                return serialize_unchecked(target, ctx);
            } else {
                return json{};
            }
        }

        json operator()(banggame::format_arg_value pair, const banggame::game_context &ctx) const {
            auto [value, type] = pair;
            switch (type) {
            case banggame::format_arg_type::format_number:
                return {{"integer", value}};
            case banggame::format_arg_type::format_card:
                return {{"card", serialize_card(value, ctx)}};
            case banggame::format_arg_type::format_player:
                return {{"player", serialize_player(value, ctx)}};
            default:
                throw serialize_error("Invalid format arg type");
            }
        }

    };

    template<> struct serializer<banggame::game_string, banggame::game_context> {
        json operator()(const banggame::game_string &value, const banggame::game_context &ctx) const {
            return {
                {"format_str", value ? std::string_view{value.format_str} : std::string_view{}},
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
                result.targets.push_back(effect.target->deserialize_target(target, context));
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
            throw lobby_error("ERROR_TIMER_EXPIRED");
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

    player_ptr game_net_manager::find_player_by_userid(int user_id) const {
        auto it = m_players_by_userid.find(user_id);
        if (it != m_players_by_userid.end()) {
            return it->second;
        }
        return nullptr;
    }

    void game_net_manager::update_player_userid(player_ptr target, int user_id) {
        if (target && target->user_id != user_id) {
            if (target->user_id != 0) {
                m_players_by_userid.erase(target->user_id);
            }
            target->user_id = user_id;
            m_players_by_userid.emplace(user_id, target);
        }
    }
}