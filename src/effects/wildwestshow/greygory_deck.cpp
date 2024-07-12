#include "greygory_deck.h"

#include "game/game.h"

#include "effects/base/can_play_card.h"

namespace banggame {
    
    static void greygory_deck_add_characters(card *target_card, player *target) {
        std::array<card *, 2> base_characters;
        rn::sample(target->m_game->get_all_cards()
            | rv::filter([&](card *c) {
                return c != target_card && c->expansion.empty()
                    && (c->pocket == pocket_type::none
                    || (c->pocket == pocket_type::player_character && c->owner == target));
            }),
            base_characters.begin(), base_characters.size(), target->m_game->rng);

        target->m_game->add_update<game_update_type::add_cards>(to_card_backface_vector(base_characters), pocket_type::player_character, target);
        for (card *c : base_characters) {
            target->m_characters.push_back(c);
            target->m_game->add_log("LOG_CHARACTER_CHOICE", target, c);
            c->pocket = pocket_type::player_character;
            c->owner = target;
            target->enable_equip(c);
            c->set_visibility(card_visibility::shown, nullptr, true);
        }
    }

    void equip_greygory_deck::on_enable(card *target_card, player *target) {
        if (target->m_characters.size() == 1) {
            greygory_deck_add_characters(target_card, target);
        }
        target->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player *origin) {
            if (origin == target) {
                target->m_game->queue_request<request_can_play_card>(target_card, nullptr, target);
            }
        });
    }
    
    void effect_greygory_deck::on_play(card *origin_card, player *origin) {
        origin->remove_extra_characters();
        greygory_deck_add_characters(origin_card, origin);
    }
}