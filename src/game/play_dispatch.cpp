#include "play_dispatch.h"

namespace banggame::play_dispatch {
    std::generator<play_card_target> possible_targets(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx) {
        co_yield std::ranges::elements_of(utils::visit_tagged([&](target_type_tag auto tag) -> std::generator<play_card_target> {
            for (auto &&value : play_visitor<tag.name>{origin, origin_card, effect}.possible_targets(ctx)) {
                co_yield play_card_target{tag, FWD(value)};
            }
        }, effect.target));
    }

    play_card_target random_target(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx) {
        return utils::visit_tagged([&](target_type_tag auto tag) {
            return play_card_target{tag, play_visitor<tag.name>{origin, origin_card, effect}.random_target(ctx)};
        }, effect.target);
    }

    game_string get_error(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target) {
        return utils::visit_tagged([&](target_type_tag auto tag, auto && ... args) {
            return play_visitor<tag.name>{origin, origin_card, effect}.get_error(ctx, FWD(args) ...);
        }, target);
    }

    prompt_string prompt(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target) {
        return utils::visit_tagged([&](target_type_tag auto tag, auto && ... args) {
            return play_visitor<tag.name>{origin, origin_card, effect}.prompt(ctx, FWD(args) ... );
        }, target);
    }

    void add_context(player_ptr origin, card_ptr origin_card, const effect_holder &effect, effect_context &ctx, const play_card_target &target) {
        utils::visit_tagged([&](target_type_tag auto tag, auto && ... args) {
            play_visitor<tag.name>{origin, origin_card, effect}.add_context(ctx, FWD(args) ... );
        }, target);
    }

    void play(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target &target) {
        utils::visit_tagged([&](target_type_tag auto tag, auto && ... args) {
            play_visitor<tag.name>{origin, origin_card, effect}.play(ctx, FWD(args) ... );
        }, target);
    }
}