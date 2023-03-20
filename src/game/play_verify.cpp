#include "play_verify.h"

#include "play_visitor.h"

#include "cards/effect_list_zip.h"
#include "cards/effect_context.h"
#include "cards/effect_enums.h"
#include "cards/game_enums.h"
#include "cards/filters.h"

#include "cards/base/requests.h"

#include "utils/raii_editor.h"
#include "utils/utils.h"

namespace banggame {

    static game_string verify_target_list(player *origin, card *origin_card, bool is_response, const target_list &targets, effect_context &ctx) {
        auto &effects = origin_card->get_effect_list(is_response);

        if (effects.empty()) {
            return "ERROR_EFFECT_LIST_EMPTY";
        }
        
        size_t diff = targets.size() - effects.size();
        if (auto repeatable = origin_card->get_tag_value(tag_type::repeatable)) {
            if (diff < 0 || diff % origin_card->optionals.size() != 0
                || (*repeatable > 0 && diff > (origin_card->optionals.size() * *repeatable)))
            {
                return "ERROR_INVALID_TARGETS";
            }
        } else if (diff != 0 && diff != origin_card->optionals.size()) {
            return "ERROR_INVALID_TARGETS";
        }

        target_list mth_targets;
        for (const auto &[target, effect] : zip_card_targets(targets, origin_card, is_response)) {
            if (!target.is(effect.target)) {
                return "ERROR_INVALID_TARGET_TYPE";
            } else if (effect.type == effect_type::mth_add) {
                mth_targets.push_back(target);
            } else if (effect.type == effect_type::ctx_add) {
                if (target.is(target_type::card)) {
                    origin_card->modifier.add_context(origin_card, origin, target.get<target_type::card>(), ctx);
                } else if (target.is(target_type::player)) {
                    origin_card->modifier.add_context(origin_card, origin, target.get<target_type::player>(), ctx);
                } else {
                    return "ERROR_INVALID_TARGET_TYPE";
                }
            }
            
            MAYBE_RETURN(enums::visit_indexed(
                [&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) -> game_string {
                    return play_visitor<E>{origin, origin_card, effect}.get_error(ctx, FWD(args) ...);
                }, target));
        }

        return origin_card->get_mth(is_response).get_error(origin_card, origin, mth_targets, ctx);
    }
    
    static game_string verify_modifiers(player *origin, card *origin_card, bool is_response, const modifier_list &modifiers, effect_context &ctx) {
        for (const auto &[mod_card, targets] : modifiers) {
            if (!mod_card->is_modifier()) {
                return "ERROR_CARD_IS_NOT_MODIFIER";
            }

            if (card *disabler = origin->m_game->get_disabler(mod_card)) {
                return {"ERROR_CARD_DISABLED_BY", mod_card.get(), disabler};
            }
            MAYBE_RETURN(origin->get_play_card_error(mod_card, ctx));

            mod_card->modifier.add_context(mod_card, origin, ctx);
            
            MAYBE_RETURN(verify_target_list(origin, mod_card, is_response, targets, ctx));
        }

        for (size_t i=0; i<modifiers.size(); ++i) {
            const auto &[mod_card, targets] = modifiers[i];

            MAYBE_RETURN(mod_card->modifier.get_error(mod_card, origin, origin_card, ctx));
            for (size_t j=0; j<i; ++j) {
                card *mod_card_before = modifiers[j].card;
                MAYBE_RETURN(mod_card_before->modifier.get_error(mod_card_before, origin, mod_card, ctx));
            }
        }
        return {};
    }

    static game_string verify_duplicates(player *origin, card *origin_card, bool is_response, const target_list &targets, const modifier_list &modifiers) {
        struct {
            std::set<card *> selected_cards;
            std::set<player *> selected_players;
            card_cube_count selected_cubes;

            game_string operator()(player *origin, card *origin_card, const effect_holder &effect, const play_card_target &target) {
                return enums::visit_indexed([&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) -> game_string {
                    auto [players, cards, cubes] = play_visitor<E>{origin, origin_card, effect}.duplicates(FWD(args) ... );
                    selected_players.merge(players);
                    if (!players.empty()) {
                        return {"ERROR_DUPLICATE_PLAYER", *players.begin()};
                    }
                    selected_cards.merge(cards);
                    if (!cards.empty()) {
                        return {"ERROR_DUPLICATE_CARD", *cards.begin()};
                    }
                    for (auto &[card, ncubes] : cubes) {
                        if (selected_cubes[card] += ncubes > card->num_cubes) {
                            return {"ERROR_NOT_ENOUGH_CUBES_ON", card};
                        }
                    }
                    return {};
                }, target);
            }
        } check;

        for (const auto &[mod_card, mod_targets] : modifiers) {
            if (!check.selected_cards.emplace(mod_card).second) {
                return {"ERROR_DUPLICATE_CARD", mod_card.get()};
            }
            for (const auto &[target, effect] : zip_card_targets(mod_targets, mod_card, is_response)) {
                MAYBE_RETURN(check(origin, mod_card, effect, target));
            }
        }

        for (const auto &[target, effect] : zip_card_targets(targets, origin_card, is_response)) {
            MAYBE_RETURN(check(origin, origin_card, effect, target));
        }

        return {};
    }

    game_string get_equip_error(player *origin, card *origin_card, player *target, const effect_context &ctx) {
        if (card *disabler = origin->m_game->get_disabler(origin_card)) {
            return {"ERROR_CARD_DISABLED_BY", origin_card, disabler};
        }
        if (origin->m_game->check_flags(game_flags::disable_equipping)) {
            return "ERROR_CANT_EQUIP_CARDS";
        }
        MAYBE_RETURN(origin->get_play_card_error(origin_card, ctx));
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

    static game_string verify_equip_target(player *origin, card *origin_card, const target_list &targets, const effect_context &ctx) {
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

    static game_string verify_card_targets(player *origin, card *origin_card, bool is_response, const target_list &targets, const modifier_list &modifiers, effect_context &ctx) {
        if (origin_card->is_modifier()) {
            return "ERROR_CARD_IS_MODIFIER";
        }
        if (card *disabler = origin->m_game->get_disabler(origin_card)) {
            return {"ERROR_CARD_DISABLED_BY", origin_card, disabler};
        }
        if (origin_card->inactive) {
            return {"ERROR_CARD_INACTIVE", origin_card};
        }
        MAYBE_RETURN(verify_modifiers(origin, origin_card, is_response, modifiers, ctx));
        MAYBE_RETURN(verify_target_list(origin, origin_card, is_response, targets, ctx));
        MAYBE_RETURN(verify_duplicates(origin, origin_card, is_response, targets, modifiers));
        MAYBE_RETURN(origin->get_play_card_error(origin_card, ctx));

        if (ctx.repeat_card != origin_card) {
            switch (origin_card->pocket) {
            case pocket_type::player_hand:
            case pocket_type::player_table:
            case pocket_type::player_character:
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

    static game_string check_prompt(player *origin, card *origin_card, bool is_response, const target_list &targets, const effect_context &ctx) {
        target_list mth_targets;
        for (const auto &[target, effect] : zip_card_targets(targets, origin_card, is_response)) {
            if (effect.type == effect_type::mth_add) {
                mth_targets.push_back(target);
            }
            MAYBE_RETURN(enums::visit_indexed([&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
                return play_visitor<E>{origin, origin_card, effect}.prompt(ctx, FWD(args) ... );
            }, target));
        }

        return origin_card->get_mth(is_response).on_prompt(origin_card, origin, mth_targets, ctx);
    }

    static game_string check_prompt_modifiers(player *origin, card *origin_card, bool is_response, const modifier_list &modifiers, const effect_context &ctx) {
        for (const auto &[mod_card, mod_targets] : modifiers) {
            MAYBE_RETURN(mod_card->modifier.on_prompt(mod_card, origin, origin_card, ctx));
            MAYBE_RETURN(check_prompt(origin, mod_card, is_response, mod_targets, ctx));
        }
        return {};
    }

    static game_string check_prompt_play(player *origin, card *origin_card, bool is_response, const target_list &targets, const modifier_list &modifiers, const effect_context &ctx) {
        MAYBE_RETURN(check_prompt_modifiers(origin, origin_card, is_response, modifiers, ctx));
        return check_prompt(origin, origin_card, is_response, targets, ctx);
    }

    static game_string check_prompt_equip(card *origin_card, player *origin, player *target, const modifier_list &modifiers, const effect_context &ctx) {
        MAYBE_RETURN(check_prompt_modifiers(origin, origin_card, false, modifiers, ctx));
        for (const auto &e : origin_card->equips) {
            MAYBE_RETURN(e.on_prompt(origin_card, origin, target));
        }
        return {};
    }

    inline void add_played_card(player *origin, card *origin_card, const modifier_list &modifiers, const effect_context &ctx) {
        if (origin_card->pocket != pocket_type::button_row) {
            origin->m_played_cards.emplace_back(origin_card, modifiers, ctx);
        }
    }

    static void log_played_card(card *origin_card, player *origin, bool is_response) {
        if (origin_card->has_tag(tag_type::skip_logs)) return;
        
        switch (origin_card->pocket) {
        case pocket_type::player_hand:
        case pocket_type::scenario_card:
        case pocket_type::hidden_deck:
            if (!origin_card->name.empty()) {
                origin->m_game->add_log(is_response ? "LOG_RESPONDED_WITH_CARD" : "LOG_PLAYED_CARD", origin_card, origin);
            }
            break;
        case pocket_type::player_table:
            origin->m_game->add_log(is_response ? "LOG_RESPONDED_WITH_CARD" : "LOG_PLAYED_TABLE_CARD", origin_card, origin);
            break;
        case pocket_type::player_character:
            origin->m_game->add_log(is_response ?
                origin_card->has_tag(tag_type::drawing)
                    ? "LOG_DRAWN_WITH_CHARACTER"
                    : "LOG_RESPONDED_WITH_CHARACTER"
                : "LOG_PLAYED_CHARACTER", origin_card, origin);
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

    void apply_target_list(player *origin, card *origin_card, bool is_response, const target_list &targets, const effect_context &ctx) {
        log_played_card(origin_card, origin, is_response);

        if (origin_card->pocket == pocket_type::player_hand) {
            origin->m_game->call_event<event_type::on_use_hand_card>(origin, origin_card, false);
        }

        if (origin_card != ctx.repeat_card && !origin_card->has_tag(tag_type::no_auto_discard)) {
            switch (origin_card->pocket) {
            case pocket_type::player_hand:
                origin->discard_card(origin_card);
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

        std::vector<std::pair<const effect_holder &, const play_card_target &>> delay_effects;
        card_cube_count selected_cubes;

        target_list mth_targets;
        for (const auto &[target, effect] : zip_card_targets(targets, origin_card, is_response)) {
            if (effect.type == effect_type::mth_add) {
                mth_targets.push_back(target);
            } else if (effect.type == effect_type::pay_cube) {
                if (auto *cs = target.get_if<target_type::select_cubes>()) {
                    for (card *c : *cs) {
                        ++selected_cubes[c];
                    }
                } else if (target.is(target_type::self_cubes)) {
                    selected_cubes[origin_card] += effect.target_value;
                }
            } else {
                delay_effects.emplace_back(effect, target);
            }
        }
        for (const auto &[c, ncubes] : selected_cubes) {
            origin->pay_cubes(c, ncubes);
        }
        for (const auto &[effect, target] : delay_effects) {
            enums::visit_indexed([&]<target_type E>(enums::enum_tag_t<E>, auto && ... args) {
                play_visitor<E>{origin, origin_card, effect}.play(ctx, FWD(args) ... );
            }, target);
        }

        origin_card->get_mth(is_response).on_play(origin_card, origin, mth_targets, ctx);
    }

    game_string verify_and_play(player *origin, card *origin_card, bool is_response, const target_list &targets, const modifier_list &modifiers) {
        if (origin->m_game->pending_requests()) {
            if (!is_response) {
                return "ERROR_MUST_RESPOND_TO_REQUEST";
            }
        } else if (is_response || origin->m_game->m_playing != origin || origin->m_game->pending_updates()) {
            return "ERROR_PLAYER_NOT_IN_TURN";
        }

        effect_context ctx;

        if (filters::is_equip_card(origin_card)) {
            if (is_response) {
                return "ERROR_INVALID_RESPONSE_CARD";
            }

            MAYBE_RETURN(verify_modifiers(origin, origin_card, is_response, modifiers, ctx));
            MAYBE_RETURN(verify_equip_target(origin, origin_card, targets, ctx));

            int cost = filters::get_card_cost(origin_card, is_response, ctx);
            if (origin->m_gold < cost) {
                return "ERROR_NOT_ENOUGH_GOLD";
            }

            player *target = origin;
            if (!origin_card->self_equippable()) {
                target = targets.front().get<target_type::player>();
            }

            origin->prompt_then(check_prompt_equip(origin_card, origin, target, modifiers, ctx), [=]{
                add_played_card(origin, origin_card, modifiers, ctx);

                origin->add_gold(-cost);
                origin_card->on_equip(target);
                for (const auto &[mod_card, mod_targets] : modifiers) {
                    apply_target_list(origin, mod_card, is_response, mod_targets, ctx);
                }
                
                origin->m_game->queue_action([=]{
                    if (origin->alive()) {
                        log_equipped_card(origin_card, origin, target);
                        
                        if (origin_card->pocket == pocket_type::player_hand) {
                            origin->m_game->call_event<event_type::on_use_hand_card>(origin, origin_card, false);
                        }

                        target->equip_card(origin_card);

                        if (origin_card->is_green()) {
                            origin_card->inactive = true;
                            origin->m_game->add_update<game_update_type::tap_card>(origin_card, true);
                        } else if (origin_card->is_black()) {
                            origin->m_game->draw_shop_card();
                        }

                        origin->m_game->call_event<event_type::on_equip_card>(origin, target, origin_card, ctx);
                    }
                });
            });
        } else {
            MAYBE_RETURN(verify_card_targets(origin, origin_card, is_response, targets, modifiers, ctx));

            int cost = filters::get_card_cost(origin_card, is_response, ctx);
            if (origin->m_gold < cost) {
                return "ERROR_NOT_ENOUGH_GOLD";
            }

            origin->prompt_then(check_prompt_play(origin, origin_card, is_response, targets, modifiers, ctx), [=]{
                add_played_card(origin, origin_card, modifiers, ctx);

                origin->add_gold(-cost);
                for (const auto &[mod_card, mod_targets] : modifiers) {
                    apply_target_list(origin, mod_card, is_response, mod_targets, ctx);
                }
                apply_target_list(origin, origin_card, is_response, targets, ctx);
            });
        }
        return {};
    }
}