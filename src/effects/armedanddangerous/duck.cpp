#include "duck.h"

#include "game/game_table.h"

#include "effects/base/bang.h"

namespace banggame {

    static int get_bang_damage(player_ptr origin) {
        if (auto req = origin->m_game->top_request<request_bang>(target_is{origin})) {
            return req->bang_damage;
        }
        return 1;
    }

    game_string effect_duck::on_prompt(card_ptr origin_card, player_ptr origin, int ncubes) const {
        if (ncubes == 0) {
            if (!origin->is_bot() || origin->m_hp > get_bang_damage(origin)) {
                return {"PROMPT_NO_REDRAW", origin_card};
            }
        }
        return {};
    }

    int effect_duck::evaluate(card_ptr origin_card, player_ptr origin, int ncubes) const {
        return ncubes;
    }

    void effect_duck::on_play(card_ptr origin_card, player_ptr origin, int ncubes) const {
        if (ncubes != 0) {
            origin->m_game->add_log("LOG_STOLEN_SELF_CARD", origin, origin_card);
            origin_card->add_short_pause();
            origin->add_to_hand(origin_card);
        }
    }
}