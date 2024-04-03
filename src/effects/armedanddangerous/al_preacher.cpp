#include "al_preacher.h"

#include "cards/game_enums.h"

#include "game/game.h"

#include "effects/base/resolve.h"

namespace banggame {

    struct request_al_preacher : request_resolvable {
        using request_resolvable::request_resolvable;

        void on_update() override {
            auto_resolve();
        }

        void on_resolve() override {
            target->m_game->pop_request();
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_CAN_PLAY_CARD", origin_card};
            } else {
                return {"STATUS_CAN_PLAY_CARD_OTHER", target, origin_card};
            }
        }
    };

    void equip_al_preacher::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_equip_card>(target_card, [=](player *origin, player *target, card *equipped_card, const effect_context &ctx) {
            if (p != origin && (equipped_card->is_blue() || equipped_card->is_orange())) {
                p->m_game->queue_request<request_al_preacher>(target_card, origin, p);
            }
        });
    }

    bool effect_al_preacher::can_play(card *origin_card, player *origin) {
        return origin->m_game->top_request<request_al_preacher>(origin) != nullptr;
    }

    void effect_al_preacher::on_play(card *origin_card, player *origin) {
        origin->m_game->pop_request();
    }

}