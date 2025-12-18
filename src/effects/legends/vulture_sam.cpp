#include "vulture_sam.h"

#include "cards/filter_enums.h"

#include "effects/base/death.h"

#include "game/game_table.h"

#include "ruleset.h"

namespace banggame {

    static card_ptr find_base_character(card_ptr origin_card) {
        std::string_view character_name = get_base_character_name(origin_card);
        for (card_ptr c : origin_card->m_game->m_characters) {
            if (c->name == character_name) {
                if (c->pocket != pocket_type::none) {
                    // handle potential edge case with vera custer:
                    // we create another copy of the card, that's fine
                    return origin_card->m_game->add_card(*c);
                }
                return c;
            }
        }
        throw game_error("Cannot find base character");
    }

    void equip_vulture_sam_legend::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_player_death>(origin_card, [=](player_ptr target, bool tried_save) {
            if (origin == target) {
                card_ptr old_character = origin->get_character();
                card_ptr target_card = find_base_character(origin_card);

                origin->disable_equip(origin_card);
                origin_card->exchange_with(target_card);

                if (origin_card == old_character) {
                    for (const auto &[token, count] : origin_card->tokens) {
                        origin->m_game->add_tokens(token, count, token_positions::card{target_card});
                    }
                    origin_card->tokens = {};
                }

                origin->enable_equip(target_card);

                origin->set_hp(old_character->get_tag_value(tag_type::max_hp).value_or(4));
            }
        });
    }
}