#include "ruleset.h"

#include "cards/game_events.h"

#include "game/game_table.h"

#include "effects/base/damage.h"

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
    }

    void effect_add_gold::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        target->add_gold(amount);
    }
}