#include "duck.h"

#include "game/game.h"
#include "game/filters.h"
#include "effects/base/bang.h"

namespace banggame {

    static int get_bang_damage(player *origin) {
        if (auto req = origin->m_game->top_request<request_bang>(origin)) {
            return req->bang_damage;
        }
        return 1;
    }

    game_string effect_duck::on_prompt(card *origin_card, player *origin, const effect_context &ctx) {
        if (filters::get_selected_cubes(origin_card, ctx).empty()) {
            if (!origin->is_bot() || origin->m_hp > get_bang_damage(origin)) {
                return {"PROMPT_NO_REDRAW", origin_card};
            }
        }
        return {};
    }

    void effect_duck::on_play(card *origin_card, player *origin, const effect_context &ctx) {
        if (!filters::get_selected_cubes(origin_card, ctx).empty()) {
            origin->m_game->add_log("LOG_STOLEN_SELF_CARD", origin, origin_card);
            origin->m_game->add_short_pause(origin_card);
            origin->add_to_hand(origin_card);
        }
    }
}