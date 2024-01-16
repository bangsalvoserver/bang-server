#include "miss_susanna.h"

#include "game/game.h"

#include "cards/effect_enums.h"

namespace banggame {

    void equip_miss_susanna::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_end>({target_card, 1}, [=](player *origin, bool skipped) {
            auto count = rn::accumulate(
                origin->m_played_cards | rv::transform([](const played_card_history &history) {
                    return rn::count_if(
                        rv::concat(
                            rv::single(history.origin_card),
                            history.modifiers
                        ),
                        [&](const card_pocket_pair &pair) {
                            return pair.pocket == pocket_type::player_hand
                                || pair.pocket == pocket_type::shop_selection
                                || pair.pocket == pocket_type::player_character && history.context.repeat_card
                                || pair.pocket == pocket_type::train && pair.origin_card->deck == card_deck_type::train
                                || pair.origin_card->get_mth(history.is_response).type == mth_type::play_as_bang;
                        }
                    );
                }), 0);
            if ((!skipped || count) && count < 3) {
                origin->damage(target_card, nullptr, 1);
            }
        });
    }
}