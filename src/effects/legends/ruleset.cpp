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
            draw_next_feat(origin);
        });
    }
}