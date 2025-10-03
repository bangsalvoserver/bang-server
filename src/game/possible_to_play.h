#ifndef __POSSIBLE_TO_PLAY_H__
#define __POSSIBLE_TO_PLAY_H__

#include "game_table.h"

#include "filters.h"
#include "play_verify.h"

#include "utils/range_utils.h"

namespace banggame {
    
    bool is_possible_to_play(player_ptr origin, card_ptr origin_card, bool is_response, card_list &modifiers, const effect_context &ctx);

    playable_cards_list generate_playable_cards_list(player_ptr origin, bool is_response = false);

    inline auto get_all_targetable_cards(player_ptr origin) {
        return rv::concat(
            origin->m_game->m_players | rv::for_each([](player_ptr target) {
                return rv::concat(target->m_hand, target->m_table, target->m_characters);
            }),
            origin->m_game->m_selection,
            origin->m_game->m_feats,
            origin->m_game->m_deck | rv::take_last(1),
            origin->m_game->m_discards | rv::take_last(1),
            origin->m_game->m_feats_deck | rv::take_last(1),
            origin->m_game->m_feats_discard | rv::take_last(1)
        );
    }

    inline auto get_all_active_cards(player_ptr origin, bool is_response, const effect_context &ctx = {}) {
        return rv::concat(
            origin->m_hand,
            origin->m_table,
            origin->m_characters,
            origin->m_game->m_button_row,
            origin->m_game->m_hidden_deck,
            origin->m_game->m_shop_selection,
            origin->m_game->m_stations,
            origin->m_game->m_train,
            origin->m_game->m_feats,
            origin->m_game->m_scenario_cards | rv::take_last(1),
            origin->m_game->m_wws_scenario_cards | rv::take_last(1),
            rv::single(ctx.repeat_card) | rv::filter([](card_ptr c) {
                return c != nullptr
                    && c->pocket != pocket_type::player_hand
                    && c->pocket != pocket_type::shop_selection;
            })
        );
    }

    inline auto get_all_playable_cards(player_ptr origin, bool is_response = false, const effect_context &ctx = {}) {
        return get_all_active_cards(origin, is_response, ctx)
            | rv::filter([=, modifiers=card_list{}](card_ptr origin_card) mutable {
                return is_possible_to_play(origin, origin_card, is_response, modifiers, ctx);
            });
    }

    inline auto get_all_equip_targets(player_ptr origin, card_ptr origin_card, const effect_context &ctx = {}) {
        return origin->m_game->m_players | rv::filter([=](player_ptr target) {
            return !get_equip_error(origin, origin_card, target, ctx);
        });
    }
}

#endif