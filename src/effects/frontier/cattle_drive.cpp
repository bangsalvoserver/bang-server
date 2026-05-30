#include "cattle_drive.h"

#include "game/game_table.h"

namespace banggame {

    game_string effect_cattle_drive::on_prompt(card_ptr origin_card, player_ptr origin) {
        if (origin->m_table.empty()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }
    
    void effect_cattle_drive::on_play(card_ptr origin_card, player_ptr origin) {
        origin->draw_card(origin->m_table.size(), origin_card);
    }

}