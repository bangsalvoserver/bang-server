#include "ruleset.h"

#include "game/game.h"

namespace banggame {

    static void draw_next_feat(player_ptr origin) {
        auto &feats_deck = origin->m_game->m_feats_deck;
        auto &feats = origin->m_game->m_feats;

        if (feats_deck.empty()) return;
        
        if (!feats.empty()) {
            origin->m_game->m_first_player->disable_equip(feats.back());
        }

        origin->m_game->add_log("LOG_DRAWN_FEAT", feats_deck.back());

        feats_deck.back()->move_to(pocket_type::feats);
        origin->m_game->m_first_player->enable_equip(feats.back());
    }

    void ruleset_legends::on_apply(game *game) {
        game->add_listener<event_type::on_game_setup>({nullptr, 0}, [](player_ptr origin) {
            card_token_type tokens[] = {
                card_token_type::fame1,
                card_token_type::fame2,
                card_token_type::fame3,
                card_token_type::fame4,
                card_token_type::fame5,
                card_token_type::fame6,
                card_token_type::fame7,
                card_token_type::fame8,
            };

            rn::shuffle(tokens, origin->m_game->rng);

            for (auto [token_type, target] : rv::zip(tokens, origin->m_game->range_all_players(origin))) {
                origin->m_game->add_tokens(token_type, card::get_max_tokens(token_type), target->first_character());
            }
            
            draw_next_feat(origin);
        });

        game->add_listener<event_type::count_initial_cards>({nullptr, -1}, [](const_player_ptr origin, int &value) {
            int count = 0;
            for (player_ptr p : origin->m_game->range_all_players(origin->m_game->m_first_player)) {
                ++count;
                if (p == origin) break;
            }
            if (count >= 6) {
                value = 5;
            } else {
                value = 4;
            }
        });

        game->add_listener<event_type::apply_maxcards_modifier>({nullptr, 20}, [](const_player_ptr origin, int &value) {
            if (origin->m_hp <= 1) {
                ++value;
            }
        });
    }

    bool ruleset_legends::is_valid_with(const expansion_set &set) {
        return set.size() == 1;
    }
}