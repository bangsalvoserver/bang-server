#include "cattle_drive.h"

#include "game/game_table.h"

namespace banggame {

    static int get_table_count(const_player_ptr target) {
        return static_cast<int>(rn::count_if(target->m_table, std::not_fn(&card::is_black)));
    }

    game_string effect_cattle_drive::on_prompt(card_ptr origin_card, player_ptr origin) {
        int count = get_table_count(origin);
        if (count == 0) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        if (origin->is_bot() && count <= 2) {
            return "BOT_DRAW_TOO_FEW_CARDS";
        }
        return {};
    }
    
    void effect_cattle_drive::on_play(card_ptr origin_card, player_ptr origin) {
        origin->draw_card(get_table_count(origin), origin_card);
    }

}