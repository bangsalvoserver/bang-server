#include "krecek.h"

#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

#include "utils/range_utils.h"

namespace banggame {

    static bool has_weapon(player_ptr p) {
        for (card_ptr c : p->m_table) {
            if (c->has_tag(tag_type::weapon)) return true;
        }
        return false;
    }

    void equip_krecek::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_discard_any_card>(target_card, [target](player_ptr origin, card_ptr discarded_card) {
            if (origin == target && discarded_card->has_tag(tag_type::weapon)) {
                target->m_game->queue_action([target]{
                    for (player_ptr other : target->m_game->range_other_players(target)) {
                        if (has_weapon(other) && other->get_weapon_range() > target->get_weapon_range()) {
                            for (card_ptr c : card_list(other->m_table)) {
                                if (c->has_tag(tag_type::weapon)) {
                                    other->m_game->add_log("LOG_DISCARDED_SELF_CARD", other, c);
                                    other->discard_card(c);
                                    break;
                                }
                            }
                        }
                    }
                }, 40);
            }
        });

        target->m_game->add_listener<event_type::on_equip_card>(target_card, [target, target_card](player_ptr origin, player_ptr owner, card_ptr equipped_card, const effect_context &ctx) {
            if (!equipped_card->has_tag(tag_type::weapon)) return;

            if (owner == target) {
                target_card->flash_card();
                target->draw_card(1, target_card);
            } else if (owner->get_weapon_range() > target->get_weapon_range()) {
                owner->m_game->add_log("LOG_DISCARDED_SELF_CARD", owner, equipped_card);
                owner->discard_card(equipped_card);
            }
        });
    }

}
