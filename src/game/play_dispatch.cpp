#include "play_dispatch.h"

namespace banggame::play_dispatch {
    std::generator<play_card_target> possible_targets(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx) {
        co_yield std::ranges::elements_of(utils::visit_tagged([&]<typename T>(std::in_place_type_t<T> tag) -> std::generator<play_card_target> {
            for (auto && value : play_visitor<T>{origin, origin_card, effect}.possible_targets(ctx)) {
                co_yield play_card_target{tag, FWD(value)};
            }
        }, effect.target));
    }

    play_card_target random_target(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx) {
        return utils::visit_tagged([&]<typename T>(std::in_place_type_t<T> tag) {
            return play_card_target{tag, play_visitor<T>{origin, origin_card, effect}.random_target(ctx)};
        }, effect.target);
    }

    game_string get_error(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target) {
        return std::visit([&]<typename T>(const T &value) {
            play_visitor<T> visitor{origin, origin_card, effect};
            if constexpr (empty_target<T>) {
                return visitor.get_error(ctx);
            } else {
                return visitor.get_error(ctx, reflect::get<0>(value));
            }
        }, target);
    }

    prompt_string prompt(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target) {
        return std::visit([&]<typename T>(const T &value) {
            play_visitor<T> visitor{origin, origin_card, effect};
            if constexpr (empty_target<T>) {
                return visitor.prompt(ctx);
            } else {
                return visitor.prompt(ctx, reflect::get<0>(value));
            }
        }, target);
    }

    void add_context(player_ptr origin, card_ptr origin_card, const effect_holder &effect, effect_context &ctx, const play_card_target &target) {
        std::visit([&]<typename T>(const T &value) {
            play_visitor<T> visitor{origin, origin_card, effect};
            if constexpr (empty_target<T>) {
                visitor.add_context(ctx);
            } else {
                visitor.add_context(ctx, reflect::get<0>(value));
            }
        }, target);
    }

    void play(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target) {
        std::visit([&]<typename T>(const T &value) {
            play_visitor<T> visitor{origin, origin_card, effect};
            if constexpr (empty_target<T>) {
                visitor.play(ctx);
            } else {
                visitor.play(ctx, reflect::get<0>(value));
            }
        }, target);
    }
}