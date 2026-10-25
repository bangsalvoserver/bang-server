#include "horous.h"

#include "cards/game_enums.h"
#include "cards/game_events.h"
#include "effects/base/draw_check.h"

#include "game/game_table.h"

namespace banggame {

    void equip_horous::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_end>(target_card, [target, target_card](player_ptr origin, bool skipped) {
            if (!skipped && origin == target) {
                target->m_game->queue_request<request_check>(target, target_card, &card_sign::is_black, [target, target_card](bool result) {
                    if (result) {
                        target_card->flash_card();
                        target->add_player_flags(player_flag::coffin);
                        target->add_player_flags(player_flag::keep_alive);

                        target->m_game->add_listener<event_type::on_turn_end>({target_card, -60}, [target, target_card](player_ptr end_origin, bool end_skipped) {
                            if (end_origin == target->get_prev_player()) {
                                target->remove_player_flags(player_flag::coffin);
                                target->remove_player_flags(player_flag::keep_alive);
                                target->m_game->remove_listeners(event_card_key{target_card, -60});
                            }
                        });
                    }
                });
            }
        });
    }

}
