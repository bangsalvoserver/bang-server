#include "play_verify.h"

#include "effects/base/requests.h"

#include "cards/game_enums.h"
#include "cards/filter_enums.h"

#include "filters.h"

#include <unordered_set>

namespace banggame::play_dispatch {
    bool possible(player *origin, card *origin_card, const effect_holder &effect, const effect_context &ctx) {
        return enums::visit_enum([&]<target_type E>(enums::enum_tag_t<E>) {
            return play_visitor<E>{origin, origin_card, effect}.possible(ctx);
        }, effect.target);
    }

    play_card_target random_target(player *origin, card *origin_card, const effect_holder &effect, const effect_context &ctx) {
        return enums::visit_enum([&]<target_type E>(enums::enum_tag_t<E> tag) -> play_card_target {
            if constexpr (play_card_target::has_type<E>) {
                return {tag, play_visitor<E>{origin, origin_card, effect}.random_target(ctx)};
            } else {
                return tag;
            }
        }, effect.target);
    }

    game_string get_error(player *origin, card *origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target) {
        return enums::visit_indexed([&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) -> game_string {
            return play_visitor<E>{origin, origin_card, effect}.get_error(ctx, FWD(args) ...);
        }, target);
    }

    game_string prompt(player *origin, card *origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target) {
        return enums::visit_indexed([&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
            return play_visitor<E>{origin, origin_card, effect}.prompt(ctx, FWD(args) ... );
        }, target);
    }

    void add_context(player *origin, card *origin_card, const effect_holder &effect, effect_context &ctx, const play_card_target &target) {
        enums::visit_indexed([&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
            play_visitor<E>{origin, origin_card, effect}.add_context(ctx, FWD(args) ... );
        }, target);
    }

    void play(player *origin, card *origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target) {
        enums::visit_indexed([&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
            play_visitor<E>{origin, origin_card, effect}.play(ctx, FWD(args) ... );
        }, target);
    }
}

namespace banggame {

    static game_string check_duplicates(const effect_context &ctx) {
        std::unordered_set<player *> players;
        for (player *p : ctx.selected_players) {
            if (auto [it, inserted] = players.insert(p); !inserted) {
                return {"ERROR_DUPLICATE_PLAYER", p};
            }
        }

        std::unordered_set<card *> cards;
        for (card *c : ctx.selected_cards) {
            if (auto [it, inserted] = cards.insert(c); !inserted) {
                return {"ERROR_DUPLICATE_CARD", c};
            }
        }

        std::unordered_map<card *, int> cubes;
        for (card *c : ctx.selected_cubes.all()) {
            if (++cubes[c] > c->num_cubes) {
                return {"ERROR_NOT_ENOUGH_CUBES_ON", c};
            }
        }
        return {};
    }

    static game_string verify_timer_response(player *origin, std::optional<timer_id_t> timer_id) {
        std::optional<timer_id_t> current_timer_id;
        if (auto req = origin->m_game->top_request()) {
            if (auto *timer = req->timer()) {
                current_timer_id = timer->get_timer_id();
            }
        }
        if (timer_id != current_timer_id) {
            return "ERROR_TIMER_EXPIRED";
        }
        return {};
    }

    static game_string verify_target_list(player *origin, card *origin_card, bool is_response, const target_list &targets, effect_context &ctx) {
        auto &effects = origin_card->get_effect_list(is_response);

        if (effects.empty()) {
            return "ERROR_EFFECT_LIST_EMPTY";
        }
        if (targets.size() != effects.size()) {
            return "ERROR_INVALID_TARGETS";
        }

        for (const auto &[target, effect] : rv::zip(targets, origin_card->get_effect_list(is_response))) {
            if (!target.is(effect.target)) {
                return "ERROR_INVALID_TARGET_TYPE";
            }

            play_dispatch::add_context(origin, origin_card, effect, ctx, target);
            
            MAYBE_RETURN(play_dispatch::get_error(origin, origin_card, effect, ctx, target));
        }

        const auto &mth = origin_card->get_mth(is_response);
        if (mth.type) {
            MAYBE_RETURN(mth.type->get_error(origin_card, origin, targets, mth.args, ctx));
        }

        MAYBE_RETURN(check_duplicates(ctx));

        if (ctx.repeat_card != origin_card) {
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
        }

        return {};
    }
    
    static game_string verify_modifiers(player *origin, card *origin_card, bool is_response, const modifier_list &modifiers, effect_context &ctx) {
        for (const auto &[mod_card, targets] : modifiers) {
            const modifier_holder &modifier = mod_card->get_modifier(is_response);

            if (modifier.type == nullptr) {
                return "ERROR_CARD_IS_NOT_MODIFIER";
            }

            ctx.selected_cards.push_back(mod_card);
            modifier.type->add_context(mod_card, origin, ctx);
            
            MAYBE_RETURN(verify_target_list(origin, mod_card, is_response, targets, ctx));
            MAYBE_RETURN(get_play_card_error(origin, mod_card, ctx));
        }

        for (size_t i=0; i<modifiers.size(); ++i) {
            const auto &[mod_card, targets] = modifiers[i];

            MAYBE_RETURN(mod_card->get_modifier(is_response).type->get_error(mod_card, origin, origin_card, ctx));
            for (size_t j=0; j<i; ++j) {
                card *mod_card_before = modifiers[j].card;
                MAYBE_RETURN(mod_card_before->get_modifier(is_response).type->get_error(mod_card_before, origin, mod_card, ctx));
            }
        }
        return {};
    }

    static game_string verify_equip_target(player *origin, card *origin_card, bool is_response, const target_list &targets, const effect_context &ctx) {
        if (is_response) {
            return "ERROR_CANNOT_EQUIP_AS_RESPONSE";
        }

        if (origin_card->pocket == pocket_type::player_hand && origin_card->owner != origin) {
            return "ERROR_INVALID_CARD_OWNER";
        }

        player *target = origin;
        if (origin_card->self_equippable()) {
            if (!targets.empty()) {
                return "ERROR_INVALID_EQUIP_TARGET";
            }
        } else {
            if (targets.size() != 1 || !targets.front().is(target_type::player)) {
                return "ERROR_INVALID_EQUIP_TARGET";
            }
            target = targets.front().get<target_type::player>();
        }
        
        return get_equip_error(origin, origin_card, target, ctx);
    }

    game_string get_play_card_error(player *origin, card *origin_card, const effect_context &ctx) {
        if (card *disabler = origin->m_game->get_disabler(origin_card)) {
            return {"ERROR_CARD_DISABLED_BY", origin_card, disabler};
        }
        if (origin_card->inactive) {
            return {"ERROR_CARD_INACTIVE", origin_card};
        }
        game_string out_error;
        origin->m_game->call_event(event_type::check_play_card{ origin, origin_card, ctx, out_error });
        return out_error;
    }

    game_string get_equip_error(player *origin, card *origin_card, player *target, const effect_context &ctx) {
        if (origin->m_game->check_flags(game_flags::disable_equipping)) {
            return "ERROR_CANT_EQUIP_CARDS";
        }
        if (origin_card->self_equippable()) {
            if (origin != target) {
                return "ERROR_INVALID_EQUIP_TARGET";
            }
        } else {
            MAYBE_RETURN(filters::check_player_filter(origin, origin_card->equip_target, target));
        }
        if (card *equipped = target->find_equipped_card(origin_card)) {
            return {"ERROR_DUPLICATED_CARD", equipped};
        }
        return {};
    }

    static game_string verify_card_targets(player *origin, card *origin_card, bool is_response, const target_list &targets, const modifier_list &modifiers, effect_context &ctx) {
        if (!is_response && origin->m_game->m_playing != origin) {
            return "ERROR_PLAYER_NOT_IN_TURN";
        }

        MAYBE_RETURN(verify_modifiers(origin, origin_card, is_response, modifiers, ctx));

        if (filters::is_equip_card(origin_card)) {
            MAYBE_RETURN(verify_equip_target(origin, origin_card, is_response, targets, ctx));
        } else if (origin_card->get_modifier(is_response).type != nullptr) {
            return "ERROR_CARD_IS_MODIFIER";
        } else {
            MAYBE_RETURN(verify_target_list(origin, origin_card, is_response, targets, ctx));
        }

        MAYBE_RETURN(get_play_card_error(origin, origin_card, ctx));
        
        if (origin->m_gold < filters::get_card_cost(origin_card, is_response, ctx)) {
            return "ERROR_NOT_ENOUGH_GOLD";
        }

        return {};
    }

    static game_string check_prompt(player *origin, card *origin_card, bool is_response, const target_list &targets, const effect_context &ctx) {
        for (const auto &[target, effect] : rv::zip(targets, origin_card->get_effect_list(is_response))) {
            MAYBE_RETURN(play_dispatch::prompt(origin, origin_card, effect, ctx, target));
        }

        const auto &mth = origin_card->get_mth(is_response);
        if (mth.type) {
            return mth.type->on_prompt(origin_card, origin, targets, mth.args, ctx);
        }
        return {};
    }

    static game_string check_prompt_play(player *origin, card *origin_card, bool is_response, const target_list &targets, const modifier_list &modifiers, const effect_context &ctx) {
        for (const auto &[mod_card, mod_targets] : modifiers) {
            MAYBE_RETURN(mod_card->get_modifier(is_response).type->on_prompt(mod_card, origin, origin_card, ctx));
            MAYBE_RETURN(check_prompt(origin, mod_card, is_response, mod_targets, ctx));
        }
        if (filters::is_equip_card(origin_card)) {
            player *target = origin_card->self_equippable() ? origin
                : targets.front().get<target_type::player>().get();
            for (const equip_holder &holder : origin_card->equips) {
                MAYBE_RETURN(holder.type->on_prompt(holder.effect_value, origin_card, origin, target));
            }
            return {};
        } else {
            return check_prompt(origin, origin_card, is_response, targets, ctx);
        }
    }

    static void log_played_card(card *origin_card, player *origin, bool is_response) {
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

    static void log_equipped_card(card *origin_card, player *origin, player *target) {
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

    static void apply_target_list(player *origin, card *origin_card, bool is_response, const target_list &targets, const effect_context &ctx) {
        log_played_card(origin_card, origin, is_response);

        if (origin_card != ctx.repeat_card && !origin_card->has_tag(tag_type::no_auto_discard)) {
            switch (origin_card->pocket) {
            case pocket_type::player_hand:
                origin->discard_used_card(origin_card);
                break;
            case pocket_type::player_table:
                if (origin_card->is_green()) {
                    origin->discard_card(origin_card);
                }
                break;
            case pocket_type::shop_selection:
                origin->m_game->move_card(origin_card, pocket_type::shop_discard);
                origin->m_game->queue_action([m_game=origin->m_game]{
                    if (m_game->m_shop_selection.size() < 3) {
                        m_game->draw_shop_card();
                    }
                }, -1);
            }
        }

        for (const auto &[target, effect] : rv::zip(targets, origin_card->get_effect_list(is_response))) {
            play_dispatch::play(origin, origin_card, effect, ctx, target);
        }

        const auto &mth = origin_card->get_mth(is_response);
        if (mth.type) {
            mth.type->on_play(origin_card, origin, targets, mth.args, ctx);
        }
    }

    static void apply_equip(player *origin, card *origin_card, const target_list &targets, const effect_context &ctx) {
        player *target = origin_card->self_equippable() ? origin
            : targets.front().get<target_type::player>().get();
            
        origin->m_game->queue_action([=]{ 
            if (!origin->alive()) return;

            log_equipped_card(origin_card, origin, target);
            
            if (origin_card->pocket == pocket_type::player_hand) {
                origin->m_game->call_event(event_type::on_discard_hand_card{ origin, origin_card, true });
            }

            target->equip_card(origin_card);

            origin->m_game->call_event(event_type::on_equip_card{ origin, target, origin_card, ctx });
        });
    }

    static played_card_history make_played_card_history(const game_action &args, bool is_response, const effect_context &ctx) {
        auto to_card_pocket_pair = [&](card *c) {
            if (ctx.repeat_card == c) {
                return card_pocket_pair{c, pocket_type::none};
            } else {
                return card_pocket_pair{c, c->pocket};
            }
        };

        return {
            .origin_card{to_card_pocket_pair(args.card)},
            .modifiers{args.modifiers
                | rv::transform(&modifier_pair::card)
                | rv::transform(to_card_pocket_pair)
                | rn::to_vector},
            .is_response{is_response},
            .context{ctx}
        };
    }

    game_message verify_and_play(player *origin, const game_action &args) {
        bool is_response = origin->m_game->pending_requests();

        effect_context ctx { .playing_card = args.card };

        if (game_string error = verify_timer_response(origin, args.timer_id)) {
            return {enums::enum_tag<message_type::error>, error};
        }

        if (game_string error = verify_card_targets(origin, args.card, is_response, args.targets, args.modifiers, ctx)) {
            return {enums::enum_tag<message_type::error>, error};
        }

        if (!args.bypass_prompt) {
            if (game_string prompt = check_prompt_play(origin, args.card, is_response, args.targets, args.modifiers, ctx)) {
                return {enums::enum_tag<message_type::prompt>, prompt};
            }
        }

        origin->m_game->send_request_status_clear();

        if (args.card->pocket != pocket_type::button_row) {
            origin->m_played_cards.push_back(make_played_card_history(args, is_response, ctx));
        }

        origin->add_gold(-filters::get_card_cost(args.card, is_response, ctx));

        for (const auto &[mod_card, mod_targets] : args.modifiers) {
            apply_target_list(origin, mod_card, is_response, mod_targets, ctx);
        }

        if (filters::is_equip_card(args.card)) {
            apply_equip(origin, args.card, args.targets, ctx);
        } else {
            apply_target_list(origin, args.card, is_response, args.targets, ctx);
        }

        return {};
    }
}