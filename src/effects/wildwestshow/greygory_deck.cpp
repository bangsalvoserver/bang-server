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

    static void add_greygory_deck_characters(card_ptr origin_card, player_ptr origin, bool allow_expansions) {
        card_list characters = sample_elements(get_possible_cards(origin_card, origin, allow_expansions), 2, origin->m_game->rng);

        origin->m_game->add_cards_to(characters, pocket_type::player_character, origin, card_visibility::shown);
        for (card_ptr c : characters) {
            origin->m_game->add_log("LOG_CHARACTER_CHOICE", origin, c);
            origin->enable_equip(c);
        }
    }

    struct request_greygory_deck : request_can_play_card {
        request_greygory_deck(card_ptr origin_card, player_ptr target, bool allow_expansions)
            : request_can_play_card(origin_card, nullptr, target)
            , allow_expansions{allow_expansions} {}

        bool allow_expansions;

        void on_update() override {
            if (target->m_characters.back() == origin_card) {
                target->m_game->pop_request();
                effect_greygory_deck{allow_expansions}.on_play(origin_card, target);
            }
        }
    };

    void equip_greygory_deck::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::check_character_modifier>({origin_card, 1}, [=, allow_expansions=allow_expansions](player_ptr target, std::set<card_ptr> &handlers) {
            if (origin == target && !handlers.contains(origin_card)
                && get_possible_cards(origin_card, origin, allow_expansions)
            ) {
                handlers.insert(origin_card);
                target->m_game->queue_request<request_greygory_deck>(origin_card, target, allow_expansions);
                return true;
            }
            return false;
        });

        origin->m_game->add_listener<event_type::on_game_setup>({origin_card, 3}, [=, allow_expansions=allow_expansions](player_ptr target) {
            add_greygory_deck_characters(origin_card, origin, allow_expansions);
        });
    }
    
    void effect_greygory_deck::on_play(card_ptr origin_card, player_ptr origin) {
        size_t start_index = rn::find(origin->m_characters, origin_card) - origin->m_characters.begin() + 1;
        origin->remove_characters(start_index);
        add_greygory_deck_characters(origin_card, origin, allow_expansions);
    }
}