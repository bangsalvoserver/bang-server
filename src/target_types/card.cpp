#include "game/play_verify.h"

#include "game/filters.h"
#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    using visit_card = play_visitor<target_type::card>;

    template<> bool visit_card::possible(const effect_context &ctx) {
        return contains_at_least(make_card_target_set(origin, origin_card, effect, ctx), 1);
    }

    template<> card *visit_card::random_target(const effect_context &ctx) {
        auto targets = make_card_target_set(origin, origin_card, effect, ctx) | rn::to_vector;
        return random_element(targets, origin->m_game->rng);
    }

    template<> game_string visit_card::get_error(const effect_context &ctx, card *target) {
        if (bool(effect.card_filter & target_card_filter::pick_card)) {
            if (auto req = origin->m_game->top_request<interface_picking>(origin)) {
                if (req->can_pick(target)) {
                    return {};
                }
            }
            return "ERROR_INVALID_PICK";
        }

        if (!target->owner) {
            return "ERROR_CARD_HAS_NO_OWNER";
        }
        MAYBE_RETURN(filters::check_player_filter(origin, effect.player_filter, target->owner, ctx));
        MAYBE_RETURN(filters::check_card_filter(origin_card, origin, effect.card_filter, target, ctx));
        return effect.type->get_error_card(effect.effect_value, origin_card, origin, target, ctx);
    }

    template<> game_string visit_card::prompt(const effect_context &ctx, card *target) {
        return effect.type->on_prompt_card(effect.effect_value, origin_card, origin, target, ctx);
    }

    template<> void visit_card::add_context(effect_context &ctx, card *target) {
        ctx.selected_cards.push_back(target);
        effect.type->add_context_card(effect.effect_value, origin_card, origin, target, ctx);
    }

    template<> void visit_card::play(const effect_context &ctx, card *target) {
        auto flags = effect_flags::single_target;
        if (origin_card->is_brown()) {
            flags |= effect_flags::escapable;
        }
        if (target->owner != origin && target->pocket == pocket_type::player_hand) {
            effect.type->on_play_card(effect.effect_value, origin_card, origin, target->owner->random_hand_card(), flags, ctx);
        } else {
            effect.type->on_play_card(effect.effect_value, origin_card, origin, target, flags, ctx);
        }
    }

}