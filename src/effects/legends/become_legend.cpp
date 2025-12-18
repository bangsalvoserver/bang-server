#include "become_legend.h"

#include "cards/game_enums.h"

#include "game/game_table.h"

#include "utils/random_element.h"

#include "ruleset.h"

namespace banggame {

    bool effect_become_legend::can_play(card_ptr origin_card, player_ptr origin) {
        if (!origin->check_player_flags(player_flag::legend)) {
            for (const auto &[token, count] : origin->tokens) {
                if (is_fame_token(token) && count != 0) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    static card_ptr find_legend_character(card_ptr origin_card) {
        auto range = origin_card->m_game->m_legends
            | rv::filter([=](card_ptr target_card) {
                std::string_view character_name = get_base_character_name(target_card);
                if (origin_card->expansion.empty()) {
                    return character_name == origin_card->name;
                } else {
                    return rn::none_of(origin_card->m_game->m_players, [&](player_ptr target) {
                        return character_name == get_base_character_name(target->get_character());
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
            origin->set_hp(std::clamp<int>(origin->m_hp, 3, origin->get_character_max_hp()));
        }
    }

    bool effect_drop_all_fame::can_play(card_ptr origin_card, player_ptr origin) {
        for (const auto &[token, count] : origin->tokens) {
            if (is_fame_token(token) && count != 0) {
                return true;
            }
        }
        return false;
    }

    void effect_drop_all_fame::on_play(card_ptr origin_card, player_ptr origin) {
        for (auto [token, count] : origin->tokens) {
            if (is_fame_token(token) && count != 0) {
                origin->m_game->add_tokens(token, -count, token_positions::player{origin});
            }
        }
    }
}