#include "miss_susanna.h"

#include "game/game.h"

namespace banggame {

    static bool is_playing_card_pair(const card_pocket_pair &pair) {
        return pair.pocket == pocket_type::player_hand
            || pair.pocket == pocket_type::shop_selection
            || pair.pocket == pocket_type::train && pair.origin_card->deck == card_deck_type::train;
    }

    void equip_miss_susanna::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_end>({target_card, 1}, [=](player *origin, bool skipped) {
            size_t count = 0;
            for (const played_card_history &history : origin->m_played_cards) {
                count += rn::count_if(history.modifiers, is_playing_card_pair);
                if (history.context.playing_card) {
                    ++count;
                } else if (is_playing_card_pair(history.origin_card)) {
                    ++count;
                }
            }
            if ((!skipped || count) && count < 3) {
                origin->damage(target_card, nullptr, 1);
            }
        });
    }
}