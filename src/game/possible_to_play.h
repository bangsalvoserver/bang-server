#ifndef __POSSIBLE_TO_PLAY_H__
#define __POSSIBLE_TO_PLAY_H__


#include "player.h"

#include "game.h"

namespace banggame {

    template<rn::input_range R> requires std::is_pointer_v<rn::range_value_t<R>>
    rn::range_value_t<R> get_single_element(R &&range) {
        auto begin = rn::begin(range);
        auto end = rn::end(range);

        if (begin != end) {
            auto first = *begin;
            if (++begin == end) {
                return first;
            }
        }
        return nullptr;
    }

    template<rn::range Rng>
    bool contains_at_least(Rng &&range, int size) {
        for (const auto &value : range) {
            if (--size == 0) return true;
        }
        return false;
    }

    struct random_element_error : std::runtime_error {
        random_element_error(): std::runtime_error{"Empty range in random_element"} {}
    };

    template<rn::range Range, typename Rng>
    decltype(auto) random_element(Range &&range, Rng &rng) {
        rn::range_value_t<Range> ret;
        if (rn::sample(std::forward<Range>(range), &ret, 1, rng).out == &ret) {
            throw random_element_error();
        }
        return ret;
    }

    template<typename T>
    using any_forward_view = rn::any_view<T, rn::category::forward>;

    any_forward_view<card *> get_all_playable_cards(player *origin, bool is_response = false);

    any_forward_view<player *> make_equip_set(player *origin, card *origin_card, const effect_context &ctx = {});

    any_forward_view<player *> make_player_target_set(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx = {});

    any_forward_view<card *> make_card_target_set(player *origin, card *origin_card, const effect_holder &holder, const effect_context &ctx = {});

    any_forward_view<player *> get_request_target_set_players(player *origin);

    any_forward_view<card *> get_request_target_set_cards(player *origin);
    
    bool is_possible_to_play(player *origin, card *origin_card, bool is_response = false, const std::vector<card *> &modifiers = {}, const effect_context &ctx = {});

    playable_cards_list generate_playable_cards_list(player *origin, bool is_response = false);
    
}

#endif