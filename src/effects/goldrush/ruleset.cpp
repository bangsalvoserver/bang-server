#include "ruleset.h"

#include "effects/base/damage.h"
#include "effects/base/death.h"
#include "effects/base/requests.h"

#include "cards/game_events.h"
#include "cards/game_enums.h"
#include "cards/filter_enums.h"

#include "game/game_table.h"
#include "game/game_options.h"

namespace banggame {

    card_ptr draw_shop_card(game_ptr game) {
        if (game->m_shop_deck.empty()) {
            throw game_error("Shop deck is empty. Cannot reshuffle");
        }
        if (game->m_shop_deck.back()->get_visibility() == card_visibility::shown) {
            for (card_ptr c : game->m_shop_deck) {
                c->visibility = {};
            }
            game->shuffle_cards_and_ids(game->m_shop_deck);
            game->add_log("LOG_SHOP_RESHUFFLED");
            game->play_sound(sound_id::shuffle);
            game->add_update(game_updates::deck_shuffled{ pocket_type::shop_deck });
        }
        card_ptr drawn_card = game->m_shop_deck.back();
        game->add_log("LOG_DRAWN_SHOP_CARD", drawn_card);
        drawn_card->move_to(pocket_type::shop_selection);
        return drawn_card;
    }

    static int get_card_cost(card_ptr target_card, const effect_context &ctx) {
        if (ctx.get<contexts::repeat_card>()) return 0;
        return target_card->get_tag_value(tag_type::buy_cost).value_or(0) - ctx.get<contexts::discount>();
    }
    
    void ruleset_goldrush::on_apply(game_ptr game) {
        game->add_listener<event_type::on_game_setup>({nullptr, 2}, [](player_ptr origin){
            origin->m_game->add_tokens(card_token_type::gold, 30, token_positions::table{});
            for (int i=0; i<3; ++i) {
                draw_shop_card(origin->m_game);
            }
        });

        game->add_listener<event_type::check_play_card>(nullptr, [](player_ptr origin, card_ptr origin_card, const effect_context &ctx, game_string &out_error) {
            if (!origin->m_game->pending_requests()) {
                if (card_ptr card_choice = ctx.get<contexts::card_choice>()) {
                    origin_card = card_choice;
                }
                if (origin_card->pocket == pocket_type::shop_selection && origin->get_gold() < get_card_cost(origin_card, ctx)) {
                    out_error = "ERROR_NOT_ENOUGH_GOLD";
                }
            }
        });
        
        game->add_listener<event_type::on_hit>({nullptr, 6}, [](card_ptr origin_card, player_ptr origin, player_ptr target, int damage, effect_flags flags) {
            if (origin && origin->m_game->m_playing == origin && origin != target && origin->alive()) {
                origin->add_gold(damage);
            }
        });

        game->add_listener<event_type::on_auto_discard>(nullptr, [](player_ptr origin, card_ptr origin_card, const effect_context &ctx) { 
            if (origin_card->pocket == pocket_type::shop_selection) {
                origin_card->move_to(pocket_type::shop_deck, nullptr, card_visibility::shown, false, pocket_position::begin);
            }
        });

        game->add_listener<event_type::on_play_card>(nullptr, [](player_ptr origin, card_ptr origin_card, const card_list &modifiers, const effect_context &ctx) {
            if (!origin->m_game->pending_requests()) {
                if (card_ptr card_choice = ctx.get<contexts::card_choice>()) {
                    origin_card = card_choice;
                }
                if (origin_card->pocket == pocket_type::shop_selection) {
                    origin->add_gold(-get_card_cost(origin_card, ctx));
                    
                    origin->m_game->queue_action([=]{
                        draw_shop_card(origin->m_game);
                    }, -1);
                }
            }
        });

        game->add_listener<event_type::on_discard_all>({nullptr, 1}, [](player_ptr origin) {
            if (origin->alive() || !origin->m_game->m_options.expansions.contains(GET_RULESET(shadowgunslingers))) {
                origin->add_gold(-origin->get_gold());
            }
        });
    }

    void effect_add_gold::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        target->add_gold(amount);
    }

    static bool only_sheriff_and_renegade_alive(player_ptr origin) {
        int alive_count = 0;
        bool has_sheriff = false;
        bool has_renegade = false;
        for (player_ptr p : origin->m_game->m_players) {
            if (!p->alive()) continue;
            ++alive_count;
            if (alive_count > 2) return false;
            if (p->m_role == player_role::sheriff) has_sheriff = true;
            else if (p->m_role == player_role::renegade) has_renegade = true;
        }
        return alive_count == 2 && has_sheriff && has_renegade;
    }

    void ruleset_shadowgunslingers::on_apply(game_ptr game) {
        game->add_listener<event_type::check_revivers>({nullptr, -2}, [](player_ptr target) {
            if (!target->alive() && !only_sheriff_and_renegade_alive(target)) {
                target->m_game->add_log("LOG_SHADOW_GUNSLINGER", target);
                target->add_player_flags(player_flag::shadow);
                target->enable_equip(target->get_character());
            }
        });
        game->add_listener<event_type::on_turn_start>({nullptr, 10}, [](player_ptr target) {
            if (target->check_player_flags(player_flag::shadow) && target->get_base_role() == player_role::renegade) {
                auto roles_revealed = target->m_game->m_players | rv::filter([](player_ptr p) {
                    return p->check_player_flags(player_flag::role_revealed);
                });
                auto count_outlaws = rn::count_if(roles_revealed, [](player_ptr p) {
                    return p->m_role == player_role::outlaw;
                });
                auto count_deputies = rn::count_if(roles_revealed, [](player_ptr p) {
                    return p->m_role == player_role::deputy || p->m_role == player_role::sheriff;
                });

                player_role role = count_deputies > count_outlaws
                    ? player_role::shadow_deputy
                    : player_role::shadow_outlaw;
                
                if (role != target->m_role) {
                    target->set_role(role);
                }
            }
        });
        game->add_listener<event_type::on_turn_end>({nullptr, -3}, [](player_ptr target, bool skipped) {
            target->m_game->queue_action([=]{
                if (target->m_extra_turns == 0 && target->remove_player_flags(player_flag::shadow) && !target->alive()) {
                    handle_player_death(nullptr, target, death_type::shadow_turn_end);
                }
            }, -10);
        });
        game->add_listener<event_type::check_remove_player>(nullptr, [](bool &value) { value = false; });
    }
}