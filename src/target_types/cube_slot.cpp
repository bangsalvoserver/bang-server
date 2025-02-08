#include "game/possible_to_play.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    using visit_card = play_visitor<"cube_slot">;

    static game_string get_cube_slot_error(card_ptr origin_card, player_ptr origin, card_ptr target, const effect_holder &effect, const effect_context &ctx) {
        if (!target->owner) return "ERROR_CARD_HAS_NO_OWNER";

        MAYBE_RETURN(check_player_filter(origin_card, origin, effect.player_filter, target->owner, ctx));

        if (target != target->owner->get_character()
            && !(target->pocket == pocket_type::player_table && target->is_orange())
        ) {
            return "ERROR_TARGET_NOT_CUBE_SLOT";
        }

        return effect.get_error(origin_card, origin, target, ctx);
    }

    static auto get_cube_slot_targets(player_ptr origin, card_ptr origin_card, const effect_holder &effect, const effect_context &ctx) {
        return origin->m_game->m_players | rv::for_each(&player::m_targetable_cards_view)
            | rv::filter([=, &ctx](card_ptr target) {
                return !get_cube_slot_error(origin_card, origin, target, effect, ctx);
            });
    }

    template<> bool visit_card::possible(const effect_context &ctx) {
        return bool(get_cube_slot_targets(origin, origin_card, effect, ctx));
    }

    template<> bool visit_card::any_of_possible_targets(const effect_context &ctx, const arg_type_predicate &fn) {
        return rn::any_of(get_cube_slot_targets(origin, origin_card, effect, ctx), fn);
    }

    template<> card_ptr visit_card::random_target(const effect_context &ctx) {
        return random_element(get_cube_slot_targets(origin, origin_card, effect, ctx), origin->m_game->bot_rng);
    }

    template<> game_string visit_card::get_error(const effect_context &ctx, card_ptr target) {
        return get_cube_slot_error(origin_card, origin, target, effect, ctx);
    }

    template<> prompt_string visit_card::prompt(const effect_context &ctx, card_ptr target) {
        return defer<"card">().prompt(ctx, target);
    }

    template<> void visit_card::add_context(effect_context &ctx, card_ptr target) {
        return defer<"card">().add_context(ctx, target);
    }

    template<> void visit_card::play(const effect_context &ctx, card_ptr target) {
        return defer<"card">().play(ctx, target);
    }

}