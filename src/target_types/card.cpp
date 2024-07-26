#include "game/possible_to_play.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    using visit_card = play_visitor<"card">;

    template<> bool visit_card::possible(const effect_context &ctx) {
        return bool(get_all_card_targets(origin, origin_card, effect, ctx));
    }

    template<> card_ptr visit_card::random_target(const effect_context &ctx) {
        return random_element(get_all_card_targets(origin, origin_card, effect, ctx), origin->m_game->bot_rng);
    }

    template<> game_string visit_card::get_error(const effect_context &ctx, card_ptr target) {
        if (target->owner) {
            MAYBE_RETURN(filters::check_player_filter(origin, effect.player_filter, target->owner, ctx));
        }
        MAYBE_RETURN(filters::check_card_filter(origin_card, origin, effect.card_filter, target, ctx));
        return effect.get_error(origin_card, origin, target, ctx);
    }

    template<> game_string visit_card::prompt(const effect_context &ctx, card_ptr target) {
        return effect.on_prompt(origin_card, origin, target, ctx);
    }

    template<> void visit_card::add_context(effect_context &ctx, card_ptr target) {
        ctx.selected_cards.push_back(target);
        effect.add_context(origin_card, origin, target, ctx);
    }

    template<> void visit_card::play(const effect_context &ctx, card_ptr target) {
        effect_flags flags = effect_flag::single_target;
        if (origin_card->is_brown()) {
            flags.add(effect_flag::escapable);
        }
        if (target->owner != origin && target->pocket == pocket_type::player_hand) {
            effect.on_play(origin_card, origin, target->owner->random_hand_card(), flags, ctx);
        } else {
            effect.on_play(origin_card, origin, target, flags, ctx);
        }
    }

}