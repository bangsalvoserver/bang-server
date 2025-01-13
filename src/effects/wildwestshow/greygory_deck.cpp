#include "greygory_deck.h"

#include "effects/base/can_play_card.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {
    
    static void greygory_deck_add_characters(card_ptr target_card, player_ptr target) {
        std::vector<card_ptr> base_characters;

        rn::sample(target->m_game->get_all_cards()
            | rv::filter([&](card_ptr c) {
                return c != target_card && c->expansion.empty()
                    && (c->pocket == pocket_type::none
                    || (c->pocket == pocket_type::player_character && c->owner == target));
            }),
            std::back_inserter(base_characters), 2, target->m_game->rng);

        target->m_game->add_cards_to(base_characters, pocket_type::player_character, target, card_visibility::shown);
        for (card_ptr c : base_characters) {
            target->m_game->add_log("LOG_CHARACTER_CHOICE", target, c);
            target->enable_equip(c);
        }
    }

    void equip_greygory_deck::on_enable(card_ptr target_card, player_ptr target) {
        if (target->m_characters.size() == 1) {
            greygory_deck_add_characters(target_card, target);
        }
        target->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player_ptr origin) {
            if (origin == target) {
                target->m_game->queue_request<request_can_play_card>(target_card, nullptr, target);
            }
        });
    }
    
    void effect_greygory_deck::on_play(card_ptr origin_card, player_ptr origin) {
        origin->remove_extra_characters();
        greygory_deck_add_characters(origin_card, origin);
    }
}