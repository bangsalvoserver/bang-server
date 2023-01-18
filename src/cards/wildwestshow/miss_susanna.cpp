#include "miss_susanna.h"

#include "game/game.h"

namespace banggame {

    void equip_miss_susanna::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_end>({target_card, 1}, [=](player *origin, bool skipped) {
            auto count = std::ranges::count_if(origin->m_played_cards
                | ranges::views::for_each([](const auto &pair) {
                    return ranges::views::concat(
                        ranges::views::single(pair.first),
                        pair.second
                    );
                }),
                [](const card_pocket_pair &pair) {
                    return pair.pocket == pocket_type::player_hand
                        || pair.pocket == pocket_type::shop_selection
                        || pair.pocket == pocket_type::hidden_deck && pair.origin_card->has_tag(tag_type::shopchoice);
                });
            if ((!skipped || count) && count < 3) {
                origin->damage(target_card, nullptr, 1);
            }
        });
    }
}