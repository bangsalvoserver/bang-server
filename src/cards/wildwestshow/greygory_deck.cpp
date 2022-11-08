#include "greygory_deck.h"

#include "game/game.h"

namespace banggame {
    
    void effect_greygory_deck::on_equip(card *target_card, player *target) {
        std::vector<card *> base_characters;
        for (card &c : target->m_game->m_cards) {
            if (c.expansion == card_expansion_type{}
                && (c.pocket == pocket_type::none
                || (c.pocket == pocket_type::player_character && c.owner == target)))
                base_characters.push_back(&c);
        }
        for (size_t i=0; i<2; ++i) {
            size_t i2 = std::uniform_int_distribution<size_t>{0, base_characters.size() - 1}(target->m_game->rng);
            std::swap(base_characters[i], base_characters[i2]);
        }
        base_characters.resize(2);

        target->m_game->add_update<game_update_type::add_cards>(make_id_vector(base_characters), pocket_type::player_character, target);
        for (card *c : base_characters) {
            target->m_characters.push_back(c);
            target->m_game->add_log("LOG_CHARACTER_CHOICE", target, c);
            c->pocket = pocket_type::player_character;
            c->owner = target;
            c->on_enable(target);
            target->m_game->send_card_update(c, target, show_card_flags::instant | show_card_flags::shown);
        }
    }
    
    void effect_greygory_deck::on_play(card *target_card, player *target) {
        target->remove_extra_characters();
        on_equip(target_card, target);
        target->m_game->update_request();
    }
}