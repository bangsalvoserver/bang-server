#include "trainarrival.h"

#include "game/game.h"

#include "cards/base/draw.h"

namespace banggame {

    void equip_trainarrival::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::count_cards_to_draw>({target_card, 1}, [](player *origin, int &value) {
            ++value;
        });
    }
}