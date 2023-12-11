#include "brothel.h"

#include "game/game.h"

#include "cards/base/deathsave.h"
#include "cards/base/predraw_check.h"

namespace banggame {

    static uint8_t brothel_counter = 0;

    void equip_brothel::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player *p, card *e_card) {
            if (p == target && e_card == target_card) {
                target->m_game->draw_check_then(target, target_card, &card_sign::is_red, [=](bool result) {
                    target->discard_card(target_card);
                    if (!result) {
                        target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        event_card_key event_key{target_card, 1 + brothel_counter++ % 20};
                        target->m_game->add_disabler(event_key, [=](card *c) {
                            return c->pocket == pocket_type::player_character && c->owner == target;
                        });
                        auto clear_events = [target, event_key](player *p) {
                            if (p == target) {
                                target->m_game->remove_disablers(event_key);
                                target->m_game->remove_listeners(event_key);
                            }
                        };
                        target->m_game->add_listener<event_type::pre_turn_start>(event_key, clear_events);
                        target->m_game->add_listener<event_type::on_player_death>(event_key, [=](player *killer, player *p) {
                            clear_events(p);
                        });
                    }
                });
            }
        });
    }
}