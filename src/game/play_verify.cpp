#include "play_verify.h"

#include "effects/base/requests.h"
#include "effects/base/equip.h"
#include "effects/base/draw.h"

#include "cards/game_enums.h"
#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "target_types/armedanddangerous/select_cubes.h"

#include "prompts.h"
#include "filters.h"
#include "game_table.h"
#include "request_timer.h"

namespace banggame {

    game_string verify_context(player_ptr origin, card_ptr origin_card, const effect_context &ctx) {
        player_set players;
        for (player_ptr p : ctx.get_all<contexts::selected_player>()) {
            if (players.contains(p)) {
                return {"ERROR_DUPLICATE_PLAYER", p};
            }
            players.add(p);
        }

        auto selected_cards = rv::concat(
            rv::single(origin_card) | rv::filter([repeat_card = ctx.get<contexts::repeat_card>()](card_ptr c) {
                return c != repeat_card;
            }),
            ctx.get_all<contexts::modifier_card>(),
            ctx.get_all<contexts::selected_card>()
        );

        card_set cards;
        for (card_ptr c : selected_cards) {
            if (cards.contains(c)) {
                return {"ERROR_DUPLICATED_CARD", c};
            }
            cards.add(c);
        }

        std::unordered_map<card_ptr, int> cubes;
        for (card_ptr c : contexts::selected_cubes::get_all_cubes(ctx)) {
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

    static game_string verify_target_list_base(player_ptr origin, card_ptr origin_card, const effect_list &effects, const target_list &targets, effect_context &ctx) {
        if (targets.size() != effects.size()) {
            return "INVALID_TARGET_LIST_SIZE";
        }

        for (const auto &[target, effect] : rv::zip(targets, effects)) {
            effect.add_context(origin_card, origin, target, ctx);
            
            if (!effect.can_play(origin_card, origin, ctx)) {
                return {"ERROR_CANT_PLAY_CARD", origin_card};
            }

            MAYBE_RETURN(effect.get_error(origin_card, origin, target, ctx));
        }

        return {};
    }

    static game_string verify_target_list(player_ptr origin, card_ptr origin_card, effect_list_type type, const target_list &targets, effect_context &ctx) {
        const effect_list &effects = origin_card->get_effect_list(type);
        if (effects.empty()) {
            return "ERROR_EFFECT_LIST_EMPTY";
        }
        
        MAYBE_RETURN(verify_target_list_base(origin, origin_card, effects, targets, ctx));

        if (const mth_holder &mth = origin_card->get_mth(type)) {
            MAYBE_RETURN(mth.get_error(origin_card, origin, targets, ctx));
        }

        if (ctx.get<contexts::repeat_card>() != origin_card) {
            MAYBE_RETURN(check_pocket(origin, origin_card));
        }

        return {};
    }
    
    static game_string verify_modifiers(player_ptr origin, card_ptr origin_card, effect_list_type type, const target_selection_list &modifiers, effect_context &ctx) {
        effect_list_type valid_type = origin->m_game->pending_requests() ? effect_list_type::responses : effect_list_type::effects;

        if (valid_type == effect_list_type::effects && origin->m_game->m_playing != origin) {
            return "ERROR_PLAYER_NOT_IN_TURN";
        }
        
        card_ptr forced_play = nullptr;

        for (const auto &[mod_card, mod_type, targets] : modifiers) {
            if (forced_play == mod_card) valid_type = effect_list_type::effects;
            
            if (valid_type != mod_type) {
                return "ERROR_INVALID_EFFECT_LIST";
            }

            if (mod_card->is_equip_card()) {
                return "ERROR_MODIFIER_IS_EQUIP";
            } else if (const modifier_holder &modifier = mod_card->get_modifier(mod_type)) {
                modifier.add_context(mod_card, origin, ctx);
                
                MAYBE_RETURN(verify_target_list(origin, mod_card, mod_type, targets, ctx));
                MAYBE_RETURN(get_play_card_error(origin, mod_card, ctx));
            } else {
                return "ERROR_CARD_IS_NOT_MODIFIER";
            }

            if (valid_type == effect_list_type::responses && !forced_play) {
                forced_play = ctx.get<contexts::forced_play>();
            }
        }

        if (forced_play == origin_card) valid_type = effect_list_type::effects;

        if (origin_card->is_equip_card()) {
            if (valid_type == effect_list_type::effects) {
                valid_type = effect_list_type::equip_effects;
            } else {
                return "ERROR_CANNOT_EQUIP_AS_RESPONSE";
            }
        } else if (origin_card->get_modifier(valid_type)) {
            return "ERROR_CARD_IS_MODIFIER";
        }

        if (valid_type != type) {
            return "ERROR_INVALID_EFFECT_LIST";
        }

        for (size_t i=0; i<modifiers.size(); ++i) {
            const auto &[mod_card, mod_type, targets] = modifiers[i];

            MAYBE_RETURN(mod_card->get_modifier(mod_type).get_error(mod_card, origin, origin_card, ctx));
            for (size_t j=0; j<i; ++j) {
                const auto &[mod_card_before, mod_type_before, targets_before] = modifiers[j];
                MAYBE_RETURN(mod_card_before->get_modifier(mod_type_before).get_error(mod_card_before, origin, mod_card, ctx));
            }
        }

        return {};
    }

    static game_string verify_equip_target(player_ptr origin, card_ptr origin_card, const target_list &targets, effect_context &ctx) {
        if (origin_card->pocket == pocket_type::player_hand && origin_card->owner != origin) {
            return "ERROR_INVALID_CARD_OWNER";
        }

        MAYBE_RETURN(verify_target_list_base(origin, origin_card, origin_card->equip_effects, targets, ctx));
        
        if (!ctx.contains<contexts::equip_target>()) {
            return effect_equip_on{}.get_error(origin_card, origin, origin, ctx);
        }

        return {};
    }

    game_string get_play_card_error(player_ptr origin, card_ptr origin_card, const effect_context &ctx) {
        if (!origin->alive() && origin_card->deck != card_deck_type::none) {
            return "ERROR_INVALID_ACTION";
        }
        if (card_ptr disabler = origin->m_game->get_disabler(origin_card)) {
            return {"ERROR_CARD_DISABLED_BY", origin_card, disabler};
        }
        return origin->m_game->call_event(event_type::check_play_card{ origin, origin_card, ctx });
    }

    static game_string verify_card_targets(player_ptr origin, card_ptr origin_card, effect_list_type type, const target_list &targets, const target_selection_list &modifiers, effect_context &ctx) {
        MAYBE_RETURN(verify_modifiers(origin, origin_card, type, modifiers, ctx));

        if (type == effect_list_type::equip_effects) {
            MAYBE_RETURN(verify_equip_target(origin, origin_card, targets, ctx));
        } else {
            MAYBE_RETURN(verify_target_list(origin, origin_card, type, targets, ctx));
        }

        MAYBE_RETURN(verify_context(origin, origin_card, ctx));

        return get_play_card_error(origin, origin_card, ctx);
    }

    static prompt_string get_play_prompt(player_ptr origin, card_ptr origin_card, effect_list_type type, const target_list &targets, const effect_context &ctx) {
        return prompts::select_prompt(rv::concat(
            rv::zip(targets, origin_card->get_effect_list(type)) | rv::transform([&](const auto &pair) {
                const auto &[target, effect] = pair;
                return effect.on_prompt(origin_card, origin, target, ctx);
            }),

            rv::single(origin_card->get_mth(type)) | rv::transform([&](const mth_holder &mth) {
                return mth ? mth.on_prompt(origin_card, origin, targets, ctx) : prompt_string{};
            })
        ));
    }

    static prompt_string get_equip_prompt(player_ptr origin, card_ptr origin_card, const target_list &targets, const effect_context &ctx) {
        return prompts::select_prompt(rv::concat(
            rv::zip(targets, origin_card->equip_effects) | rv::transform([&](const auto &pair) {
                const auto &[target, effect] = pair;
                return effect.on_prompt(origin_card, origin, target, ctx);
            }),

            rv::single(!ctx.contains<contexts::equip_target>()
                ? effect_equip_on{}.on_prompt(origin_card, origin, origin, ctx) : prompt_string{})
        ));
    }

    static prompt_string get_prompt_message(player_ptr origin, card_ptr origin_card, effect_list_type type, const target_list &targets, const target_selection_list &modifiers, const effect_context &ctx) {
        return prompts::select_prompt(rv::concat(
            modifiers | rv::transform([&](const target_selection &selection) {
                return get_play_prompt(origin, selection.card, selection.effect_list, selection.targets, ctx);
            }),

            rv::single(type == effect_list_type::equip_effects
                ? get_equip_prompt(origin, origin_card, targets, ctx)
                : get_play_prompt(origin, origin_card, type, targets, ctx)
            )
        ));
    }

    static void log_played_card(card_ptr origin_card, player_ptr origin, effect_list_type type, const effect_context &ctx) {
        if (origin_card->has_tag(tag_type::skip_logs)) return;
        
        switch (origin_card->pocket) {
        case pocket_type::player_hand:
        case pocket_type::scenario_card:
            if (type == effect_list_type::responses) {
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
            if (type == effect_list_type::responses) {
                origin->m_game->add_log("LOG_RESPONDED_WITH_CARD", origin_card, origin);
            } else {
                origin->m_game->add_log("LOG_PLAYED_TABLE_CARD", origin_card, origin);
            }
            break;
        case pocket_type::player_character:
            if (ctx.contains<contexts::drawing_effect>()) {
                origin->m_game->add_log("LOG_DRAWN_WITH_CHARACTER", origin_card, origin);
            } else if (type == effect_list_type::responses) {
                origin->m_game->add_log("LOG_RESPONDED_WITH_CHARACTER", origin_card, origin);
            } else {
                origin->m_game->add_log("LOG_PLAYED_CHARACTER", origin_card, origin);
            }
            break;
        case pocket_type::shop_selection:
            if (origin_card == ctx.get<contexts::forced_play>()) {
                origin->m_game->add_log("LOG_PLAYED_CARD", origin_card, origin);
            } else {
                origin->m_game->add_log("LOG_BOUGHT_CARD", origin_card, origin);
            }
            break;
        case pocket_type::stations:
            origin->m_game->add_log("LOG_PAID_FOR_STATION", origin_card, origin);
            break;
        }
    }

    static void apply_target_list(player_ptr origin, card_ptr origin_card, effect_list_type type, const target_list &targets, const effect_context &ctx) {
        log_played_card(origin_card, origin, type, ctx);

        if (!rn::contains(ctx.get_all<contexts::auto_discarded>(), origin_card)) {
            if (origin_card->pocket == pocket_type::player_hand) {
                origin->discard_used_card(origin_card);
            } else {
                origin->m_game->call_event(event_type::on_auto_discard{ origin, origin_card, ctx });
            }
        }

        for (const auto &[target, effect] : rv::zip(targets, origin_card->get_effect_list(type))) {
            effect.on_play(origin_card, origin, target, ctx);
        }

        if (const mth_holder &mth = origin_card->get_mth(type)) {
            mth.on_play(origin_card, origin, targets, ctx);
        }
    }

    static void apply_equip(player_ptr origin, card_ptr origin_card, const target_list &targets, const effect_context &ctx) {
        for (const auto &[target, effect] : rv::zip(targets, origin_card->equip_effects)) {
            effect.on_play(origin_card, origin, target, ctx);
        }

        if (!ctx.contains<contexts::equip_target>()) {
            effect_equip_on{}.on_play(origin_card, origin, origin, ctx);
        }
    }

    play_verify_result verify_and_play(player_ptr origin, const game_action &args) {
        effect_context ctx;
        ctx.add(contexts::playing_card{ args.card });

        if (game_string error = verify_card_targets(origin, args.card, args.effect_list, args.targets, args.modifiers, ctx)) {
            return play_verify_results::error{ error };
        }

        if (!args.bypass_prompt) {
            if (prompt_string prompt = get_prompt_message(origin, args.card, args.effect_list, args.targets, args.modifiers, ctx)) {
                return play_verify_results::prompt{ prompt };
            }
        }

        origin->m_game->clear_request_status();

        origin->m_game->call_event(event_type::on_play_card{ origin, args.card, ctx });

        for (const auto &[mod_card, mod_key, mod_targets] : args.modifiers) {
            apply_target_list(origin, mod_card, mod_key, mod_targets, ctx);
        }

        if (args.effect_list == effect_list_type::equip_effects) {
            apply_equip(origin, args.card, args.targets, ctx);
        } else {
            apply_target_list(origin, args.card, args.effect_list, args.targets, ctx);
        }

        return play_verify_results::ok{};
    }
}