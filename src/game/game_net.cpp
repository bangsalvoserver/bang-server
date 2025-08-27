#include "game_net.h"

#include "cards/card_serial.h"
#include "cards/expansion_set.h"
#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "request_timer.h"
#include "play_verify.h"

#include "net/lobby.h"

namespace json {
    
    template<typename Context>
    struct serializer<banggame::tag_map, Context> {
        static void write(const banggame::tag_map &map, string_writer &writer) {
            writer.StartObject();
            for (const auto &[k, v] : map) {
                auto key = enums::to_string(k);
                writer.Key(key.data(), key.size());
                writer.Int(v);
            }
            writer.EndObject();
        }
    };

    template<typename Context>
    struct serializer<const banggame::effect_vtable *, Context> {
        static void write(const banggame::effect_vtable *value, string_writer &writer) {
            writer.String(value->name.data(), value->name.size());
        }
    };

    template<typename Context>
    struct serializer<const banggame::targeting_vtable *, Context> {
        static void write(const banggame::targeting_vtable *value, string_writer &writer) {
            writer.String(value->name.data(), value->name.size());
        }
    };

    template<> struct serializer<const void *, banggame::game_context> {
        struct skip_field{};
    };

    template<> struct serializer<banggame::effect_holder, banggame::game_context> : aggregate_serializer_unchecked<banggame::effect_holder, banggame::game_context> {
        static void write(const banggame::effect_holder &effect, string_writer &writer, const banggame::game_context &ctx) {
            writer.StartObject();
            write_fields(effect, writer, ctx);
            effect.target->serialize_args(effect, writer, ctx);
            writer.EndObject();
        }
    };

    template<typename Context>
    struct serializer<const banggame::equip_vtable *, Context> {
        static void write(const banggame::equip_vtable *value, string_writer &writer) {
            writer.String(value->name.data(), value->name.size());
        }
    };

    template<typename Context>
    struct serializer<const banggame::modifier_vtable *, Context> {
        static void write(const banggame::modifier_vtable *value, string_writer &writer) {
            if (value) {
                writer.String(value->name.data(), value->name.size());
            } else {
                writer.Null();
            }
        }
    };

    template<typename Context> struct serializer<const banggame::mth_vtable *, Context> {
        static void write(const banggame::mth_vtable *value, string_writer &writer) {
            if (value) {
                writer.String(value->name.data(), value->name.size());
            } else {
                writer.Null();
            }
        }
    };

    template<typename Context> struct serializer<banggame::card_backface_list, Context> {
        struct card_backface {
            int id;
            banggame::card_deck_type deck;
        };

        static void write(const banggame::card_backface_list &value, string_writer &writer, const Context &ctx) {
            auto to_card_backface = [](banggame::const_card_ptr card) {
                return card_backface{ card->id, card->deck };
            };

            serialize(value.cards | rv::transform(to_card_backface), writer, ctx);
        }
    };

    template<typename Context>
    struct serializer<banggame::player_user_list, Context> {
        struct player_user_pair {
            int player_id;
            int user_id;
        };

        static void write(const banggame::player_user_list &value, string_writer &writer, const Context &ctx) {
            auto to_player_user_pair = [](banggame::const_player_ptr player) {
                return player_user_pair{ player->id, player->user_id };
            };

            serialize(value.players | rv::transform(to_player_user_pair), writer, ctx);
        };
    };

    template<> struct serializer<banggame::format_arg_value, banggame::game_context> {
        static void serialize_int(int value, string_writer &writer) {
            writer.StartObject();
            writer.Key("integer");
            writer.Int(value);
            writer.EndObject();
        }

        static void serialize_card(int card_id, string_writer &writer, const banggame::game_context &ctx){
            struct format_card {
                std::string_view name;
                banggame::card_sign sign;
            };

            writer.StartObject();
            writer.Key("card");
            writer.StartObject();
            if (card_id != 0) {
                banggame::card_ptr target_card = ctx.find_card(card_id);
                using serializer_type = aggregate_serializer_unchecked<format_card, banggame::game_context>;
                serializer_type::write_fields({ target_card->name, target_card->sign }, writer, ctx);
            }
            writer.EndObject();
            writer.EndObject();
        }

        static void serialize_player(int player_id, string_writer &writer, const banggame::game_context &ctx) {
            writer.StartObject();
            writer.Key("player");
            if (player_id != 0) {
                banggame::player_ptr target = ctx.find_player(player_id);
                serialize(target, writer, ctx);
            } else {
                writer.Null();
            }
            writer.EndObject();
        }

        static void write(banggame::format_arg_value pair, string_writer &writer, const banggame::game_context &ctx) {
            auto [value, type] = pair;
            switch (type) {
            case banggame::format_arg_type::format_number:
                serialize_int(value, writer);
                break;
            case banggame::format_arg_type::format_card:
                serialize_card(value, writer, ctx);
                break;
            case banggame::format_arg_type::format_player:
                serialize_player(value, writer, ctx);
                break;
            default:
                throw serialize_error("Invalid format arg type");
            }
        }

    };

    template<> struct serializer<banggame::game_string, banggame::game_context> {
        static void write(const banggame::game_string &value, string_writer &writer, const banggame::game_context &ctx) {
            writer.StartObject();
            
            writer.Key("format_str");
            writer.String(value.format_str ? value.format_str : "");
            
            writer.Key("format_args");
            serialize(value.format_args, writer, ctx);

            writer.EndObject();
        }
    };

    template<> struct serializer<banggame::animation_duration, banggame::game_context> {
        static void write(const banggame::animation_duration &duration, string_writer &writer, const banggame::game_context &context) {
            serialize(context.transform_duration(duration.get()), writer, context);
        }
    };

}

namespace banggame {

    static card_targets_pair deserialize_card_targets(const json::json &card, const json::json &targets, const game_context &context, bool check_equip = false) {
        card_targets_pair result {
            .card = json::deserialize<card_ptr, game_context>(card, context)
        };

        if (!targets.IsArray()) {
            throw json::deserialize_error("Cannot deserialize target list: value is not an array");
        }

        if (check_equip && result.card->is_equip_card()) {
            if (result.card->self_equippable()) {
                if (!targets.Empty()) {
                    throw json::deserialize_error("Self equippable card must have no targets");
                }
            } else {
                if (targets.Size() != 1) {
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

            if (effects.size() != targets.Size()) {
                throw json::deserialize_error("Invalid number of targets");
            }
            
            result.targets.reserve(effects.size());
            for (const auto &[effect, target] : rv::zip(effects, targets.GetArray())) {
                result.targets.push_back(effect.target->deserialize_target(target, context));
            }
        }

        return result;
    }

    static const json::json &get_value(const json::json &obj, std::string_view key) {
        json::json json_key(rapidjson::StringRef(key.data(), key.size()));
        if (auto it = obj.FindMember(json_key); it != obj.MemberEnd()) {
            return it->value;
        }
        throw json::deserialize_error(std::format("Missing key {} in object", key));
    }

    static modifier_list deserialize_modifier_list(const json::json &value, const game_context &context) {
        if (!value.IsArray()) {
            throw json::deserialize_error("Cannot deserialize modifier_list: value is not an array");
        }
        modifier_list result;
        result.reserve(value.Size());
        for (const json::json &modifier : value.GetArray()) {
            if (!modifier.IsObject()) {
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

    json::raw_string game_net_manager::serialize_update(const game_update &update) const {
        return json::to_string<game_update, game_context>(update, *this);
    }

    void game_net_manager::handle_game_action(player_ptr origin, const json::json &value) {
        if (!value.IsObject()) {
            throw json::deserialize_error("Cannot deserialize game_action: value is not an object");
        }
        
        std::optional<timer_id_t> timer_id, current_timer_id;
        if (auto it = value.FindMember("timer_id"); it != value.MemberEnd()) {
            timer_id = json::deserialize<timer_id_t>(it->value);
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