#include "miss_susanna.h"

#include "game/game.h"

namespace banggame {

    void equip_miss_susanna::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_end>({target_card, 1}, [=](player *origin, bool skipped) {
            auto count = ranges::count(origin->m_played_cards
                | ranges::views::transform([](const auto &tup) {
                    return ranges::views::concat(
                        ranges::views::single(std::get<pocket_type>(tup)),
                        std::get<std::vector<pocket_type>>(tup)
                    );
                })
                | ranges::views::join,
                pocket_type::player_hand);
            if ((!skipped || count) && count < 3) {
                origin->damage(target_card, nullptr, 1);
            }
        });
    }
}