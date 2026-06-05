#ifndef __POSSIBLE_TO_PLAY_H__
#define __POSSIBLE_TO_PLAY_H__

#include "game_table.h"

#include "filters.h"
#include "play_verify.h"

#include "utils/random_element.h"

namespace banggame {

    class possible_to_play {
    private:
        player_ptr origin;
        effect_context ctx;
        modifier_list modifiers;
        target_list targets;
    
    public:
        possible_to_play(player_ptr origin, const effect_context &ctx = {})
            : origin{origin}, ctx{ctx} {}
    
    private:
        bool check_recurse(card_ptr origin_card, effect_list_type type, size_t skip_targets);
        bool check_impl(card_ptr origin_card, effect_list_type &type);

        void collect_recurse(card_ptr origin_card, effect_list_type type, playable_cards_list &result);

    public:
        bool check(card_ptr origin_card, effect_list_type type = effect_list_type::effects) {
            return check_impl(origin_card, type);
        }

        friend playable_cards_list generate_playable_cards_list(player_ptr origin, effect_list_type type);
    };

    playable_cards_list generate_playable_cards_list(player_ptr origin, effect_list_type type = effect_list_type::effects);

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

    inline auto get_all_active_cards(player_ptr origin, const effect_context &ctx = {}) {
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
            rv::single(ctx.get<contexts::repeat_card>()) | rv::filter([=](card_ptr c) {
                return c != nullptr
                    && (c->pocket != pocket_type::player_hand || c->owner != origin)
                    && c->pocket != pocket_type::shop_selection;
            })
        );
    }

    inline auto get_all_playable_cards(player_ptr origin, effect_list_type type = effect_list_type::effects, const effect_context &ctx = {}) {
        return get_all_active_cards(origin, ctx)
            | rv::filter([type, state=possible_to_play{origin, ctx}](card_ptr origin_card) mutable {
                return state.check(origin_card, type);
            });
    }
}

#endif