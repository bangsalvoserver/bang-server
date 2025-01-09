#include "become_legend.h"

#include "cards/game_enums.h"
#include "game/game_table.h"

namespace banggame {

    bool effect_become_legend::can_play(card_ptr origin_card, player_ptr origin) {
        if (!origin->check_player_flags(player_flag::legend)) {
            for (const auto &[token, count] : origin->first_character()->tokens) {
                if (token != card_token_type::cube && count != 0) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    static card_ptr find_legend_character(card_ptr origin_card) {
        auto range = origin_card->m_game->get_all_cards()
            | rv::filter([&](card_ptr target_card) {
                if (target_card->deck != card_deck_type::legends) {
                    return false;
                } else if (origin_card->expansion.empty()) {
                    return target_card->name == origin_card->name;
                } else {
                    return rn::none_of(origin_card->m_game->m_players, [&](player_ptr target) {
                        return target->first_character()->name == target_card->name;
                    });
                }
            });
        if (range) {
            return random_element(range, origin_card->m_game->rng);
        }
        throw game_error("Cannot find legend character");
    }

    void effect_become_legend::on_play(card_ptr origin_card, player_ptr origin) {
        origin->add_player_flags(player_flag::legend);

        origin->remove_extra_characters();
        for (card_ptr c : origin->m_characters) {
            origin->disable_equip(c);
            c->visibility = card_visibility::hidden;
        }

        card_ptr old_character = origin->first_character();
        card_ptr legend_character = find_legend_character(old_character);

        origin->m_game->add_log("LOG_BECOME_LEGEND", origin, legend_character);

        origin->m_game->add_update<"remove_cards">(origin->m_characters);
        origin->m_characters = std::vector{ legend_character };
        origin->m_game->add_update<"add_cards">(origin->m_characters, pocket_type::player_character, origin);

        legend_character->pocket = pocket_type::player_character;
        legend_character->owner = origin;
        legend_character->set_visibility(card_visibility::shown, origin, true);
                
        for (const auto &[token, count] : old_character->tokens) {
            old_character->move_tokens(token, legend_character, count, true);
        }

        origin->reset_max_hp();
        origin->enable_equip(legend_character);

        legend_character->flash_card();
        
        if (origin->m_hp < 3) {
            origin->set_hp(3);
        }
    }
}