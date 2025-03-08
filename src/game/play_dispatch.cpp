#include "play_dispatch.h"

namespace banggame::play_dispatch {
    bool any_of_possible_targets(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx, const play_card_target_predicate &fn) {
        return utils::visit_tagged([&](target_type_tag auto tag) {
            play_visitor<tag.name> visitor{origin, origin_card, effect};
            if constexpr (!std::is_void_v<target_type_value<decltype(tag)>>) {
                return visitor.any_of_possible_targets(ctx, [&](auto &&arg) {
                    return fn(play_card_target{tag, FWD(arg)});
                });
            } else {
                return visitor.any_of_possible_targets(ctx, [&] {
                    return fn(play_card_target{tag});
                });
            }
        }, effect.target);
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