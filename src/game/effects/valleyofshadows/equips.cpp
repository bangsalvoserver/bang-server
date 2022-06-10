#include "equips.h"
#include "requests.h"

#include "../base/requests.h"

#include "../../game.h"

namespace banggame {

    void effect_snake::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player *p, card *e_card) {
            if (p == target && e_card == target_card) {
                target->m_game->draw_check_then(target, target_card, [=](card *drawn_card) {
                    if (target->get_card_sign(drawn_card).suit == card_suit::spades) {
                        target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        target->damage(target_card, nullptr, 1);
                    }
                });
            }
        });
    }

    void effect_ghost::on_equip(card *target_card, player *target) {
        target->add_player_flags(player_flags::targetable);
        for (card *c : target->m_characters) {
            target->enable_equip(c);
        }
    }

    void effect_ghost::on_enable(card *target_card, player *target) {
        target->add_player_flags(player_flags::ghost);
    }

    void effect_ghost::on_disable(card *target_card, player *target) {
        target->remove_player_flags(player_flags::ghost);
    }
    
    void effect_ghost::on_unequip(card *target_card, player *target) {
        target->remove_player_flags(player_flags::targetable);
        target->m_game->queue_action_front([=]{
            target->m_game->handle_player_death(nullptr, target);
        });
    }

    void effect_shotgun::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_hit>({target_card, 4}, [=](card *origin_card, player *origin, player *target, int damage, bool is_bang) {
            if (origin == p && target != p && is_bang) {
                target->m_game->queue_action([=]{
                    if (target->alive() && !target->m_hand.empty()) {
                        target->m_game->queue_request<request_discard>(target_card, origin, target);
                    }
                });
            }
        });
    }

    void effect_bounty::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_hit>({target_card, 3}, [=](card *origin_card, player *origin, player *target, int damage, bool is_bang) {
            if (origin && target == p && is_bang) {
                origin->draw_card(1, target_card);
            }
        });
    }
}