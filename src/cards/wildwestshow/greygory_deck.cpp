#include "greygory_deck.h"

#include "game/game.h"

namespace banggame {
    
    static void greygory_deck_add_characters(card *target_card, player *target) {
        std::array<card *, 2> base_characters;
        rn::sample(target->m_game->m_context.cards
            | rv::transform([](card &c) { return &c; })
            | rv::filter([&](card *c) {
                return c != target_card && c->expansion == expansion_type{}
                    && (c->pocket == pocket_type::none
                    || (c->pocket == pocket_type::player_character && c->owner == target));
            }),
            base_characters.begin(), base_characters.size(), target->m_game->rng);

        target->m_game->add_update<game_update_type::add_cards>(rn::to<std::vector<card_backface>>(base_characters), pocket_type::player_character, target);
        for (card *c : base_characters) {
            target->m_characters.push_back(c);
            target->m_game->add_log("LOG_CHARACTER_CHOICE", target, c);
            c->pocket = pocket_type::player_character;
            c->owner = target;
            target->enable_equip(c);
            target->m_game->set_card_visibility(c, nullptr, card_visibility::shown, true);
        }
    }

    void equip_greygory_deck::on_enable(card *target_card, player *target) {
        if (target->m_characters.size() == 1) {
            greygory_deck_add_characters(target_card, target);
        }    
    }
    
    void effect_greygory_deck::on_play(card *target_card, player *target) {
        target->remove_extra_characters();
        greygory_deck_add_characters(target_card, target);
    }
}