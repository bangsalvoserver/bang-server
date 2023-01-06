#include "miss_susanna.h"

#include "game/game.h"

namespace banggame {

    void equip_miss_susanna::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_end>({target_card, 1}, [=](player *origin, bool skipped) {
            auto count = ranges::count_if(origin->m_played_cards
                | ranges::views::transform([](const auto &pair) {
                    return ranges::views::concat(ranges::views::single(pair.first), pair.second);
                })
                | ranges::views::join,
                [](card *c) {
                    return c->pocket != pocket_type::button_row && c->sign;
                }
            );
            if ((!skipped || count) && count < 3) {
                origin->damage(target_card, nullptr, 1);
            }
        });
    }
}