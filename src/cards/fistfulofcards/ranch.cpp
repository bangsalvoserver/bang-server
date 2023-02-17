#include "ranch.h"

#include "game/game.h"

namespace banggame {

    struct request_ranch : request_base {
        request_ranch(card *target_card, player *target)
            : request_base(target_card, nullptr, target, effect_flags::auto_respond) {}

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_RANCH", origin_card};
            } else {
                return {"STATUS_RANCH_OTHER", origin_card, target};
            }
        }
    };

    void equip_ranch::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_draw_from_deck>({target_card, -1}, [=](player *origin) {
            origin->m_game->queue_request<request_ranch>(target_card, origin);
        });
    }

    bool effect_ranch::can_play(card *origin_card, player *origin) {
        return origin->m_game->top_request<request_ranch>(origin) != nullptr;
    }

    void effect_ranch::on_play(card *origin_card, player *origin) {
        origin->m_game->pop_request();
    }
}