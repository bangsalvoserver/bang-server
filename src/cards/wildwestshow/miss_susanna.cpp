#include "miss_susanna.h"

#include "game/game.h"

namespace banggame {

    void equip_miss_susanna::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_end>({target_card, 1}, [=](player *origin, bool skipped) {
            auto count = std::ranges::count_if(origin->m_played_cards
                | ranges::views::for_each([](const played_card_history &pair) {
                    return ranges::views::concat(
                        ranges::views::single(pair.origin_card),
                        pair.modifiers
                    );
                }),
                [](const card_pocket_pair &pair) {
                    return pair.pocket == pocket_type::player_hand
                        || pair.pocket == pocket_type::shop_selection
                        || pair.pocket == pocket_type::train && pair.origin_card->deck == card_deck_type::train;
                });
            if ((!skipped || count) && count < 3) {
                origin->damage(target_card, nullptr, 1);
            }
        });
    }
}