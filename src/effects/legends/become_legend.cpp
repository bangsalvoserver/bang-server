#include "become_legend.h"

#include "cards/game_enums.h"
#include "game/game_table.h"

namespace banggame {

    bool effect_become_legend::can_play(card_ptr origin_card, player_ptr origin) {
        if (!origin->check_player_flags(player_flag::legend)) {
            for (const auto &[token, count] : origin->get_character()->tokens) {
                if (token != card_token_type::cube && count != 0) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    static card_ptr find_legend_character(card_ptr origin_card) {
        auto range = origin_card->m_game->get_deck(card_deck_type::legends)
            | rv::filter([=](card_ptr target_card) {
                if (origin_card->expansion.empty()) {
                    return target_card->name == origin_card->name;
                } else {
                    return rn::none_of(origin_card->m_game->m_players, [&](player_ptr target) {
                        return target->get_character()->name == target_card->name;
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

        card_ptr legend_character = find_legend_character(origin->get_character());
        
        origin->m_game->add_log("LOG_BECOME_LEGEND", origin, legend_character);
        origin->set_character(legend_character);

        legend_character->flash_card();
        
        if (!origin->is_ghost()) {
            origin->set_hp(std::clamp<int>(origin->m_hp, 3, origin->m_max_hp));
        }
    }
}