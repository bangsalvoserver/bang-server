#ifndef __POSSIBLE_TO_PLAY_H__
#define __POSSIBLE_TO_PLAY_H__

#include "game.h"

#include "filters.h"
#include "play_verify.h"

#include "utils/range_utils.h"

namespace banggame {
    
    bool is_possible_to_play(player *origin, card *origin_card, bool is_response = false, const std::vector<card *> &modifiers = {}, const effect_context &ctx = {});

    playable_cards_list generate_playable_cards_list(player *origin, bool is_response = false);
    
    inline auto get_all_active_cards(player *origin) {
        return rv::concat(
            origin->m_hand,
            origin->m_table,
            origin->m_characters,
            origin->m_game->m_button_row,
            origin->m_game->m_hidden_deck,
            origin->m_game->m_shop_selection,
            origin->m_game->m_stations,
            origin->m_game->m_train,
            origin->m_game->m_scenario_cards | rv::take_last(1),
            origin->m_game->m_wws_scenario_cards | rv::take_last(1)
        );
    }

    inline auto get_all_targetable_cards(player *origin) {
        return rv::concat(
            origin->m_game->m_players | rv::for_each(&player::m_targetable_cards_view),
            origin->m_game->m_selection,
            origin->m_game->m_deck | rv::take(1),
            origin->m_game->m_discards | rv::take(1)
        );
    }

    inline auto get_all_playable_cards(player *origin, bool is_response = false) {
        return rv::filter(get_all_active_cards(origin), [=](card *origin_card) {
            return is_possible_to_play(origin, origin_card, is_response);
        });
    }

    inline auto get_all_equip_targets(player *origin, card *origin_card, const effect_context &ctx = {}) {
        return rv::filter(origin->m_game->m_players, [=](player *target) {
            return !get_equip_error(origin, origin_card, target, ctx);
        });
    }

    inline auto get_all_player_targets(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx = {}) {
        return rv::filter(origin->m_game->m_players, [=](player *target) {
            return !filters::check_player_filter(origin, holder.player_filter, target, ctx)
                && !holder.get_error(origin_card, origin, target, ctx);
        });
    }

    inline auto get_all_card_targets(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx = {}) {
        return rv::filter(get_all_targetable_cards(origin), [=](card *target_card) {
            return (!target_card->owner || !filters::check_player_filter(origin, holder.player_filter, target_card->owner, ctx))
                && !filters::check_card_filter(origin_card, origin, holder.card_filter, target_card, ctx)
                && !holder.get_error(origin_card, origin, target_card, ctx);
        });
    }
}

#endif