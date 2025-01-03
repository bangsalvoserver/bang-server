#include "vulture_sam.h"

#include "effects/base/deathsave.h"

#include "game/game.h"

namespace banggame {

    static card_ptr find_base_character(card_ptr origin_card) {
        for (card_ptr c : origin_card->m_game->get_all_cards()) {
            if (c->deck == card_deck_type::character && c->name == origin_card->name) {
                return c;
            }
        }
        throw game_error("Cannot find base character");
    }

    void equip_vulture_sam_legend::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_player_death>(origin_card, [=](player_ptr target, bool tried_save) {
            if (origin == target) {
                origin->remove_extra_characters();
                for (card_ptr c : origin->m_characters) {
                    origin->disable_equip(c);
                }

                card_ptr base_character = find_base_character(origin_card);

                origin->m_game->add_update<"remove_cards">(origin->m_characters);
                origin->m_characters = std::vector{ base_character };
                origin->m_game->add_update<"add_cards">(origin->m_characters, pocket_type::player_character, origin);

                base_character->pocket = pocket_type::player_character;
                base_character->owner = origin;
                base_character->set_visibility(card_visibility::shown, origin, true);
                        
                for (const auto &[token, count] : origin_card->tokens) {
                    origin_card->move_tokens(token, base_character, count, true);
                }

                origin->reset_max_hp();
                origin->enable_equip(base_character);
                
                origin->set_hp(4);
            }
        });
    }
}