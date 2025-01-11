#include "ruleset.h"

#include "effects/base/damage.h"
#include "effects/base/death.h"
#include "effects/base/requests.h"

#include "cards/game_events.h"
#include "cards/game_enums.h"

#include "game/game_table.h"
#include "game/game_options.h"

namespace banggame {

    card_ptr draw_shop_card(game_ptr game) {
        if (game->m_shop_deck.empty()) {
            throw game_error("Shop deck is empty. Cannot reshuffle");
        }
        if (game->m_shop_deck.back()->visibility == card_visibility::shown) {
            for (card_ptr c : game->m_shop_deck) {
                c->visibility = card_visibility::hidden;
            }
            game->shuffle_cards_and_ids(game->m_shop_deck);
            game->add_log("LOG_SHOP_RESHUFFLED");
            game->play_sound("shuffle");
            game->add_update<"deck_shuffled">(pocket_type::shop_deck);
        }
        card_ptr drawn_card = game->m_shop_deck.back();
        game->add_log("LOG_DRAWN_SHOP_CARD", drawn_card);
        drawn_card->move_to(pocket_type::shop_selection);
        return drawn_card;
    }
    
    void ruleset_goldrush::on_apply(game_ptr game) {
        game->add_listener<event_type::on_game_setup>({nullptr, 2}, [=](player_ptr origin){
            for (int i=0; i<3; ++i) {
                draw_shop_card(game);
            }
        });
        
        game->add_listener<event_type::on_hit>({nullptr, 6}, [=](card_ptr origin_card, player_ptr origin, player_ptr target, int damage, effect_flags flags) {
            if (origin && game->m_playing == origin && origin != target && origin->alive()) {
                origin->add_gold(damage);
            }
        });

        game->add_listener<event_type::on_play_card>(nullptr, [=](player_ptr origin, card_ptr origin_card, const card_list &modifiers, const effect_context &ctx) {
            if (ctx.card_choice) {
                origin_card = ctx.card_choice;
            }
            if (origin_card->pocket == pocket_type::shop_selection) {
                game->queue_action([=]{ draw_shop_card(game); }, -1);
            }
        });

        game->add_listener<event_type::on_discard_all>({nullptr, 1}, [=](player_ptr origin) {
            if (origin->alive() || !game->m_options.expansions.contains(GET_RULESET(shadowgunslingers))) {
                origin->add_gold(-origin->m_gold);
            }
        });
    }

    void effect_add_gold::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        target->add_gold(amount);
    }

    void ruleset_shadowgunslingers::on_apply(game_ptr game) {
        game->add_listener<event_type::check_revivers>({nullptr, -2}, [=](player_ptr target) {
            if (!target->alive()) {
                game->add_log("LOG_SHADOW_GUNSLINGER", target);
                target->add_player_flags(player_flag::shadow);
                for (card_ptr c : target->m_characters) {
                    target->enable_equip(c);
                }
            }
        });
        game->add_listener<event_type::on_turn_start>({nullptr, 10}, [=](player_ptr target) {
            if (target->check_player_flags(player_flag::shadow) && target->get_base_role() == player_role::renegade) {
                auto roles_revealed = game->m_players | rv::filter([](player_ptr p) {
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
                    target->set_role(player_role::unknown, false);
                    target->set_role(role, false);
                }
            }
        });
        game->add_listener<event_type::on_turn_end>({nullptr, -3}, [=](player_ptr target, bool skipped) {
            game->queue_action([=]{
                if (target->m_extra_turns == 0 && target->remove_player_flags(player_flag::shadow) && !target->alive()) {
                    handle_player_death(nullptr, target, death_type::shadow_turn_end);
                }
            }, -10);
        });
        game->add_listener<event_type::check_remove_player>(nullptr, [](bool &value) {
            value = false;
        });
    }
}