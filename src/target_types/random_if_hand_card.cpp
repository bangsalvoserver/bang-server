#include "game/possible_to_play.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    using visit_card = play_visitor<"random_if_hand_card">;

    template<> std::generator<card_ptr> visit_card::possible_targets(const effect_context &ctx) {
        co_yield std::ranges::elements_of(defer<"card">().possible_targets(ctx));
    }

    template<> card_ptr visit_card::random_target(const effect_context &ctx) {
        return defer<"card">().random_target(ctx);
    }

    template<> game_string visit_card::get_error(const effect_context &ctx, card_ptr target) {
        return defer<"card">().get_error(ctx, target);
    }

    template<> prompt_string visit_card::prompt(const effect_context &ctx, card_ptr target) {
        return defer<"card">().prompt(ctx, target);
    }

    template<> void visit_card::add_context(effect_context &ctx, card_ptr target) {
        return defer<"card">().add_context(ctx, target);
    }

    template<> void visit_card::play(const effect_context &ctx, card_ptr target) {
        if (target->pocket == pocket_type::player_hand) {
            target = target->owner->random_hand_card();
        }
        defer<"card">().play(ctx, target);
    }

}