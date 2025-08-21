#include "play_verify.h"

#include "effects/base/requests.h"

#include "cards/game_enums.h"
#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "filters.h"
#include "game_table.h"
#include "request_timer.h"

#include <unordered_set>

namespace banggame {

    game_string check_duplicates(const effect_context &ctx) {
        std::unordered_set<player_ptr> players;
        for (player_ptr p : ctx.selected_players) {
            if (auto [it, inserted] = players.insert(p); !inserted) {
                return {"ERROR_DUPLICATE_PLAYER", p};
            }
        }

        std::unordered_set<card_ptr> cards;
        for (card_ptr c : ctx.selected_cards) {
            if (auto [it, inserted] = cards.insert(c); !inserted) {
                return {"ERROR_DUPLICATE_CARD", c};
            }
        }

        std::unordered_map<card_ptr, int> cubes;
        for (card_ptr c : ctx.selected_cubes.all_cubes()) {
            if (++cubes[c] > c->num_cubes()) {
                return {"ERROR_NOT_ENOUGH_CUBES_ON", c};
            }
        }
        return {};
    }

    static game_string check_pocket(player_ptr origin, card_ptr origin_card) {
        switch (origin_card->pocket) {
        case pocket_type::player_hand:
        case pocket_type::player_table:
        case pocket_type::player_character:
            if (origin_card->owner != origin) {
                return "ERROR_INVALID_CARD_OWNER";
            }
            break;
        case pocket_type::button_row:
        case pocket_type::shop_selection:
        case pocket_type::hidden_deck:
        case pocket_type::stations:
        case pocket_type::train:
        case pocket_type::feats:
            break;
        case pocket_type::scenario_card:
            if (origin_card != origin->m_game->m_scenario_cards.back()) {
                return "ERROR_INVALID_SCENARIO_CARD";
            }
            break;
        case pocket_type::wws_scenario_card:
            if (origin_card != origin->m_game->m_wws_scenario_cards.back()) {
                return "ERROR_INVALID_SCENARIO_CARD";
            }
            break;
        default:
            return "ERROR_INVALID_CARD_POCKET";
        }

        return {};
    }

    static game_string verify_target_list(player_ptr origin, card_ptr origin_card, bool is_response, const target_list &targets, effect_context &ctx) {
        auto &effects = origin_card->get_effect_list(is_response);

        for (const auto &[target, effect] : rv::zip(targets, effects)) {
            effect.add_context(origin_card, origin, target, ctx);
            
            if (!effect.can_play(origin_card, origin, ctx)) {
                return {"ERROR_CANT_PLAY_CARD", origin_card};
            }

            MAYBE_RETURN(effect.get_error(origin_card, origin, target, ctx));
        }

        if (const mth_holder &mth = origin_card->get_mth(is_response)) {
            MAYBE_RETURN(mth.get_error(origin_card, origin, targets, ctx));
        }

        MAYBE_RETURN(check_duplicates(ctx));

        if (ctx.repeat_card != origin_card) {
            MAYBE_RETURN(check_pocket(origin, origin_card));
        }

        return {};
    }
    
    static game_string verify_modifiers(player_ptr origin, card_ptr origin_card, bool is_response, const modifier_list &modifiers, effect_context &ctx) {
        for (const auto &[mod_card, targets] : modifiers) {
            if (const modifier_holder &modifier = mod_card->get_modifier(is_response)) {
                ctx.selected_cards.push_back(mod_card);
                modifier.add_context(mod_card, origin, ctx);
                
                MAYBE_RETURN(verify_target_list(origin, mod_card, is_response, targets, ctx));
                MAYBE_RETURN(get_play_card_error(origin, mod_card, ctx));
            } else {
                return "ERROR_CARD_IS_NOT_MODIFIER";
            }
        }

        for (size_t i=0; i<modifiers.size(); ++i) {
            const auto &[mod_card, targets] = modifiers[i];

            MAYBE_RETURN(mod_card->get_modifier(is_response).get_error(mod_card, origin, origin_card, ctx));
            for (size_t j=0; j<i; ++j) {
                card_ptr mod_card_before = modifiers[j].card;
                MAYBE_RETURN(mod_card_before->get_modifier(is_response).get_error(mod_card_before, origin, mod_card, ctx));
            }
        }
        return {};
    }

    static player_ptr get_equip_target(player_ptr origin, card_ptr origin_card, const target_list &targets) {
        return origin_card->self_equippable() ? origin : targets.front().get<player_ptr>();
    }

    static game_string verify_equip_target(player_ptr origin, card_ptr origin_card, bool is_response, const target_list &targets, const effect_context &ctx) {
        if (is_response) {
            return "ERROR_CANNOT_EQUIP_AS_RESPONSE";
        }

        if (origin_card->pocket == pocket_type::player_hand && origin_card->owner != origin) {
            return "ERROR_INVALID_CARD_OWNER";
        }

        return get_equip_error(origin, origin_card, get_equip_target(origin, origin_card, targets), ctx);
    }

    game_string get_play_card_error(player_ptr origin, card_ptr origin_card, const effect_context &ctx) {
        if (card_ptr disabler = origin->m_game->get_disabler(origin_card)) {
            return {"ERROR_CARD_DISABLED_BY", origin_card, disabler};
        }
        game_string out_error;
        origin->m_game->call_event(event_type::check_play_card{ origin, origin_card, ctx, out_error });
        return out_error;
    }

    game_string get_equip_error(player_ptr origin, card_ptr origin_card, const_player_ptr target, const effect_context &ctx) {
        if (origin->m_game->check_flags(game_flag::disable_equipping)) {
            return "ERROR_CANT_EQUIP_CARDS";
        }
        if (origin_card->self_equippable()) {
            if (origin != target) {
                return "ERROR_INVALID_EQUIP_TARGET";
            }
        } else {
            MAYBE_RETURN(check_player_filter(origin_card, origin, origin_card->equip_target, target));
        }
        if (card_ptr equipped = target->find_equipped_card(origin_card)) {
            return {"ERROR_DUPLICATED_CARD", equipped};
        }
        return {};
    }

    static game_string verify_card_targets(player_ptr origin, card_ptr origin_card, bool is_response, const target_list &targets, const modifier_list &modifiers, effect_context &ctx) {
        if (!is_response && origin->m_game->m_playing != origin) {
            return "ERROR_PLAYER_NOT_IN_TURN";
        }

        MAYBE_RETURN(verify_modifiers(origin, origin_card, is_response, modifiers, ctx));

        if (origin_card->is_equip_card()) {
            MAYBE_RETURN(verify_equip_target(origin, origin_card, is_response, targets, ctx));
        } else if (origin_card->get_modifier(is_response)) {
            return "ERROR_CARD_IS_MODIFIER";
        } else {
            MAYBE_RETURN(verify_target_list(origin, origin_card, is_response, targets, ctx));
        }

        return get_play_card_error(origin, origin_card, ctx);
    }

    prompt_string get_equip_prompt(player_ptr origin, card_ptr origin_card, player_ptr target) {
        for (const equip_holder &holder : origin_card->equips) {
            MAYBE_RETURN(holder.on_prompt(origin_card, origin, target));
        }
        return {};
    }

    static prompt_string get_play_prompt(player_ptr origin, card_ptr origin_card, bool is_response, const target_list &targets, const effect_context &ctx) {
        for (const auto &[target, effect] : rv::zip(targets, origin_card->get_effect_list(is_response))) {
            MAYBE_RETURN(effect.on_prompt(origin_card, origin, target, ctx));
        }

        if (const mth_holder &mth = origin_card->get_mth(is_response)) {
            return mth.on_prompt(origin_card, origin, targets, ctx);
        }
        return {};
    }

    static prompt_string get_prompt_message(player_ptr origin, card_ptr origin_card, bool is_response, const target_list &targets, const modifier_list &modifiers, const effect_context &ctx) {
        for (const auto &[mod_card, mod_targets] : modifiers) {
            MAYBE_RETURN(get_play_prompt(origin, mod_card, is_response, mod_targets, ctx));
        }
        if (origin_card->is_equip_card()) {
            return get_equip_prompt(origin, origin_card, get_equip_target(origin, origin_card, targets));
        } else {
            return get_play_prompt(origin, origin_card, is_response, targets, ctx);
        }
    }

    static void log_played_card(card_ptr origin_card, player_ptr origin, bool is_response) {
        if (origin_card->has_tag(tag_type::skip_logs)) return;
        
        switch (origin_card->pocket) {
        case pocket_type::player_hand:
        case pocket_type::scenario_card:
            if (is_response) {
                origin->m_game->add_log("LOG_RESPONDED_WITH_CARD", origin_card, origin);
            } else {
                origin->m_game->add_log("LOG_PLAYED_CARD", origin_card, origin);
            }
            break;
        case pocket_type::hidden_deck:
            if (origin_card->has_tag(tag_type::card_choice)) {
                origin->m_game->add_log("LOG_CHOSE_CARD", origin_card, origin);
            }
            break;
        case pocket_type::player_table:
            if (is_response) {
                origin->m_game->add_log("LOG_RESPONDED_WITH_CARD", origin_card, origin);
            } else {
                origin->m_game->add_log("LOG_PLAYED_TABLE_CARD", origin_card, origin);
            }
            break;
        case pocket_type::player_character:
            if (is_response) {
                if (origin_card->has_tag(tag_type::drawing)) {
                    origin->m_game->add_log("LOG_DRAWN_WITH_CHARACTER", origin_card, origin);
                } else {
                    origin->m_game->add_log("LOG_RESPONDED_WITH_CHARACTER", origin_card, origin);
                }
            } else {
                origin->m_game->add_log("LOG_PLAYED_CHARACTER", origin_card, origin);
            }
            break;
        case pocket_type::shop_selection:
            origin->m_game->add_log("LOG_BOUGHT_CARD", origin_card, origin);
            break;
        case pocket_type::stations:
            origin->m_game->add_log("LOG_PAID_FOR_STATION", origin_card, origin);
            break;
        }
    }

    static void log_equipped_card(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (origin_card->pocket == pocket_type::shop_selection) {
            if (origin == target) {
                origin->m_game->add_log("LOG_BOUGHT_EQUIP", origin_card, origin);
            } else {
                origin->m_game->add_log("LOG_BOUGHT_EQUIP_TO", origin_card, origin, target);
            }
        } else {
            if (origin == target) {
                origin->m_game->add_log("LOG_EQUIPPED_CARD", origin_card, origin);
            } else {
                origin->m_game->add_log("LOG_EQUIPPED_CARD_TO", origin_card, origin, target);
            }
        }
    }

    static void apply_target_list(player_ptr origin, card_ptr origin_card, bool is_response, const target_list &targets, const effect_context &ctx) {
        log_played_card(origin_card, origin, is_response);

        if (origin_card != ctx.repeat_card && !origin_card->has_tag(tag_type::no_auto_discard)) {
            if (origin_card->pocket == pocket_type::player_hand) {
                origin->discard_used_card(origin_card);
            } else {
                origin->m_game->call_event(event_type::on_auto_discard{ origin, origin_card, ctx });
            }
        }

        for (const auto &[target, effect] : rv::zip(targets, origin_card->get_effect_list(is_response))) {
            effect.on_play(origin_card, origin, target, ctx);
        }

        if (const mth_holder &mth = origin_card->get_mth(is_response)) {
            mth.on_play(origin_card, origin, targets, ctx);
        }
    }

    static void apply_equip(player_ptr origin, card_ptr origin_card, player_ptr target, const effect_context &ctx) {
        origin->m_game->queue_action([=]{ 
            if (!origin->alive()) return;

            log_equipped_card(origin_card, origin, target);
            
            if (origin_card->pocket == pocket_type::player_hand) {
                origin->m_game->call_event(event_type::on_discard_hand_card{ origin, origin_card, true });
            }

            target->equip_card(origin_card);

            origin->m_game->call_event(event_type::on_equip_card{ origin, target, origin_card, ctx });
        }, 45);
    }

    play_verify_result verify_and_play(player_ptr origin, const game_action &args) {
        bool is_response = origin->m_game->pending_requests();

        effect_context ctx{};
        
        ctx.playing_card = args.card;

        if (game_string error = verify_card_targets(origin, args.card, is_response, args.targets, args.modifiers, ctx)) {
            return play_verify_results::error{ error };
        }

        if (!args.bypass_prompt) {
            if (prompt_string prompt = get_prompt_message(origin, args.card, is_response, args.targets, args.modifiers, ctx)) {
                return play_verify_results::prompt{ prompt };
            }
        }

        origin->m_game->clear_request_status();

        origin->m_game->call_event(event_type::on_play_card{
            origin,
            args.card,
            args.modifiers | rv::transform(&card_targets_pair::card) | rn::to<std::vector>(),
            ctx
        });

        for (const auto &[mod_card, mod_targets] : args.modifiers) {
            apply_target_list(origin, mod_card, is_response, mod_targets, ctx);
        }

        if (args.card->is_equip_card()) {
            apply_equip(origin, args.card, get_equip_target(origin, args.card, args.targets), ctx);
        } else {
            apply_target_list(origin, args.card, is_response, args.targets, ctx);
        }

        return play_verify_results::ok{};
    }
}