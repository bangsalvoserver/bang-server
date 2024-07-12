#include "taxman.h"

#include "effects/base/draw.h"
#include "effects/base/predraw_check.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void equip_taxman::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::get_predraw_checks>({target_card, 10},
            [=, priority=predraw_check_priority](player *origin, std::vector<event_card_key> &ret) {
                if (origin == target && !target->m_game->check_flags(game_flag::phase_one_override)) {
                    ret.emplace_back(target_card, priority);
                };
            });

        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player *p, card *e_card) {
            if (p == target && e_card == target_card) {
                target->m_game->queue_request<request_check>(target, target_card, &card_sign::is_red, [=](bool result) {
                    if (!result) {
                        target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        
                        event_card_key key{target_card, 1};
                        target->m_game->add_listener<event_type::count_cards_to_draw>(key, [=](player *origin, int &value) {
                            if (origin == target) {
                                --value;
                            }
                        });
                        target->m_game->add_listener<event_type::on_turn_end>(key, [=](player *origin, bool skipped) {
                            if (origin == target) {
                                origin->m_game->remove_listeners(key);
                            }
                        });
                    }
                });
            }
        });
    }
}