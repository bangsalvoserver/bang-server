#include "greygory_deck.h"

#include "effects/base/can_play_card.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_greygory_deck::on_enable(card_ptr target_card, player_ptr target) {
        if (target->m_characters.size() == 1) {
            effect_greygory_deck{allow_expansions}.on_play(target_card, target);
        }
        target->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player_ptr origin) {
            if (origin == target) {
                target->m_game->queue_request<request_can_play_card>(target_card, nullptr, target);
            }
        });
    }
    
    void effect_greygory_deck::on_play(card_ptr origin_card, player_ptr origin) {
        origin->remove_cards({origin->m_characters.begin() + 1, origin->m_characters.end()});
        card_list characters = sample_elements(origin->m_game->m_characters
            | rv::filter([&](card_ptr c) {
                return c->pocket == pocket_type::none
                    && (c->owner == nullptr || c->owner == origin)
                    && (allow_expansions || c->expansion.empty())
                    && c->name != origin_card->name;
            }),
            2, origin->m_game->rng
        );

        origin->m_game->add_cards_to(characters, pocket_type::player_character, origin, card_visibility::shown);
        for (card_ptr c : characters) {
            origin->m_game->add_log("LOG_CHARACTER_CHOICE", origin, c);
            origin->enable_equip(c);
        }
    }
}