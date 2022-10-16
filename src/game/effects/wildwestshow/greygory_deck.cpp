#include "greygory_deck.h"

#include "../../game.h"

namespace banggame {
    
    void effect_greygory_deck::on_equip(card *target_card, player *target) {
        std::vector<card *> base_characters;
        for (card &c : target->m_game->m_cards) {
            if (c.expansion == card_expansion_type{}
                && (c.pocket == pocket_type::none
                || (c.pocket == pocket_type::player_character && c.owner == target)))
                base_characters.push_back(&c);
        }
        std::ranges::shuffle(base_characters, target->m_game->rng);

        target->m_game->add_update<game_update_type::add_cards>(
            make_id_vector(base_characters | std::views::take(2)),
            pocket_type::player_character, target);
        for (int i=0; i<2; ++i) {
            auto *c = target->m_characters.emplace_back(base_characters[i]);
            target->m_game->add_log("LOG_CHARACTER_CHOICE", target, c);
            c->pocket = pocket_type::player_character;
            c->owner = target;
            c->on_enable(target);
            target->m_game->send_card_update(c, target, show_card_flags::instant | show_card_flags::shown);
        }
    }
    
    void effect_greygory_deck::on_play(card *target_card, player *target) {
        for (int i=1; i<target->m_characters.size(); ++i) {
            auto *c = target->m_characters[i];
            target->disable_equip(c);
            c->pocket = pocket_type::none;
            c->owner = nullptr;
        }
        target->remove_extra_characters();
        on_equip(target_card, target);
        target->m_game->update_request();
    }
}