#include "spike_spiezel.h"

#include "cards/filter_enums.h"

#include "effects/wildwestshow/leevankliff.h"

#include "game/game.h"

namespace banggame {

    game_string modifier_spike_spiezel::get_error(card_ptr origin_card, player_ptr origin, card_ptr playing_card, const effect_context &ctx) {
        if (!ctx.repeat_card) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
        }

        playing_card = get_repeat_playing_card(playing_card, ctx);
        
        if (ctx.repeat_card != playing_card) {
            return "INVALID_MODIFIER_CARD";
        }

        if (!playing_card->is_brown()) {
            return "ERROR_CARD_IS_NOT_BROWN";
        }

        return {};
    }

    void modifier_spike_spiezel::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
        auto range = origin->m_played_cards
            | rv::reverse
            | rv::transform([](const played_card_history &history) {
                return get_repeat_playing_card(history.origin_card.origin_card, history.context);
            })
            | rv::cache1
            | rv::filter(&card::is_brown);
        if (auto it = range.begin(); it != range.end()) {
            ctx.disable_banglimit = true;
            ctx.repeat_card = *it;
        }
    }
}