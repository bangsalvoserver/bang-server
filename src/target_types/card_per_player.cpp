#include "game/possible_to_play.h"

#include "cards/game_enums.h"

namespace banggame {

    using visit_cards = play_visitor<"card_per_player">;

    template<> bool visit_cards::possible(const effect_context &ctx) {
        return true;
    }

    template<> bool visit_cards::any_of_possible_targets(const effect_context &ctx, const arg_type_predicate &fn) {
        return true;
    }

    static auto cards_target_set(const_player_ptr origin, const_card_ptr origin_card, enums::bitset<target_card_filter> filter, player_ptr target, const effect_context &ctx) {
        return rv::concat(target->m_table, target->m_hand)
            | rv::filter([=, &ctx](const_card_ptr target_card) {
                return !check_card_filter(origin_card, origin, filter, target_card, ctx);
            });
    }

    template<> card_list visit_cards::random_target(const effect_context &ctx) {
        card_list ret;
        for (player_ptr target : origin->m_game->range_alive_players(origin)) {
            if (target != ctx.skipped_player && !check_player_filter(origin_card, origin, effect.player_filter, target, ctx)) {
                if (auto targets = cards_target_set(origin, origin_card, effect.card_filter, target, ctx)) {
                    ret.push_back(random_element(targets, origin->m_game->bot_rng));
                }
            }
        }
        return ret;
    }

    template<> game_string visit_cards::get_error(const effect_context &ctx, const card_list &target_cards) {
        if (!rn::all_of(origin->m_game->m_players, [&](player_ptr p) {
            size_t found = rn::count(target_cards, p, &card::owner);
            if (p == ctx.skipped_player || check_player_filter(origin_card, origin, effect.player_filter, p, ctx)) return found == 0;
            if (cards_target_set(origin, origin_card, effect.card_filter, p, ctx).empty()) return found == 0;
            return found == 1;
        })) {
            return "ERROR_INVALID_TARGETS";
        } else {
            for (card_ptr c : target_cards) {
                MAYBE_RETURN(check_card_filter(origin_card, origin, effect.card_filter, c, ctx));
                MAYBE_RETURN(effect.get_error(origin_card, origin, c, ctx));
            }
            return {};
        }
    }

    template<> prompt_string visit_cards::prompt(const effect_context &ctx, const card_list &target_cards) {
        if (target_cards.empty()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return merge_prompts(target_cards | rv::transform([&](card_ptr target_card) {
            return effect.on_prompt(origin_card, origin, target_card, ctx);
        }));
    }

    template<> void visit_cards::add_context(effect_context &ctx, const card_list &target_cards) {
        for (card_ptr target_card : target_cards) {
            effect.add_context(origin_card, origin, target_card, ctx);
        }
    }

    template<> void visit_cards::play(const effect_context &ctx, const card_list &target_cards) {
        effect_flags flags = effect_flag::multi_target;
        if (target_cards.size() == 1) {
            flags.add(effect_flag::single_target);
        }
        for (card_ptr target_card : target_cards) {
            if (target_card->pocket == pocket_type::player_hand) {
                effect.on_play(origin_card, origin, target_card->owner->random_hand_card(), flags, ctx);
            } else {
                effect.on_play(origin_card, origin, target_card, flags, ctx);
            }
        }
    }

}