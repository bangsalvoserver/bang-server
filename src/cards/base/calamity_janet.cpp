#include "calamity_janet.h"

#include "cards/game_enums.h"

#include "game/game.h"

#include "missed.h"

namespace banggame {

    void equip_calamity_janet::on_enable(card *origin_card, player *p) {
        p->add_player_flags(player_flags::treat_missed_as_bang);
    }

    void equip_calamity_janet::on_disable(card *origin_card, player *p) {
        p->remove_player_flags(player_flags::treat_missed_as_bang);
    }

    game_string handler_play_as_missed::get_error(card *origin_card, player *origin, card *target_card) {
        if (effect_missed{}.can_play(target_card, origin)) {
            return {};
        }
        return "ERROR_INVALID_ACTION";
    }

    game_string handler_play_as_missed::on_prompt(card *origin_card, player *origin, card *target_card) {
        return effect_missed{}.on_prompt(target_card, origin);
    }

    void handler_play_as_missed::on_play(card *origin_card, player *origin, card *target_card) {
        origin->m_game->add_log("LOG_PLAYED_CARD_AS_MISSED", target_card, origin);
        origin->discard_used_card(target_card);
        effect_missed{}.on_play(target_card, origin);
    }
}