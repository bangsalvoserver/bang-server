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

    game_string effect_duck::on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        if (ctx.get<contexts::selected_cubes>().count(origin_card) == 0) {
            if (!origin->is_bot() || origin->m_hp > get_bang_damage(origin)) {
                return {"PROMPT_NO_REDRAW", origin_card};
            }
        }
        return {};
    }

    void effect_duck::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        if (ctx.get<contexts::selected_cubes>().count(origin_card) != 0) {
            origin->m_game->add_log("LOG_STOLEN_SELF_CARD", origin, origin_card);
            origin_card->add_short_pause();
            origin->add_to_hand(origin_card);
        }
    }
}