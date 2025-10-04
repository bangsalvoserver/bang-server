#include "greygory_deck.h"

#include "effects/base/can_play_card.h"

#include "cards/game_events.h"

#include "game/game_table.h"

#include "utils/random_element.h"

namespace banggame {
	
    static auto get_possible_cards(card_ptr origin_card, player_ptr origin, bool allow_expansions) {
        return origin->m_game->m_characters | rv::filter([=](card_ptr c) {
            return c->pocket == pocket_type::none
                && c->owner == nullptr
                && c->name != origin_card->name
                && (allow_expansions || c->expansion.empty());
        });
    }

    void equip_greygory_deck::on_enable(card_ptr origin_card, player_ptr origin) {
        if (origin->m_characters.size() == 1) {
            effect_greygory_deck{allow_expansions}.on_play(origin_card, origin);
        }
        origin->m_game->add_listener<event_type::check_character_modifier>({origin_card, 1}, [=, allow_expansions=allow_expansions](player_ptr target, bool &handled, std::set<card_ptr> &handlers) {
            if (origin == target && !handled && !handlers.contains(origin_card)
                && get_possible_cards(origin_card, origin, allow_expansions)
            ) {
                handled = true;
                handlers.insert(origin_card);
                target->m_game->queue_request<request_can_play_card>(origin_card, nullptr, target);
            }
        });
    }
    
    void effect_greygory_deck::on_play(card_ptr origin_card, player_ptr origin) {
        origin->remove_cards({origin->m_characters.begin() + 1, origin->m_characters.end()});
        
        auto possible_cards = get_possible_cards(origin_card, origin, allow_expansions);
        card_list characters = sample_elements(possible_cards, 2, origin->m_game->rng);

        origin->m_game->add_cards_to(characters, pocket_type::player_character, origin, card_visibility::shown);
        for (card_ptr c : characters) {
            origin->m_game->add_log("LOG_CHARACTER_CHOICE", origin, c);
            origin->enable_equip(c);
        }
    }
}