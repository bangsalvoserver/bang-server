#include "tuco_franziskaner.h"

#include "game/game_table.h"

#include "effects/base/draw.h"

namespace banggame {

    void equip_tuco_franziskaner::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::init_request_draw>({target_card, 1}, [p](player_ptr origin, shared_request_draw req) {
            if (p == origin && rn::none_of(p->m_table, &card::is_blue)) {
                req->num_cards_to_draw += 2;
            }
        });
    }
}