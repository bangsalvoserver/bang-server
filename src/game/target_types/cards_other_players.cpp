#include "game/play_verify.h"

#include "cards/game_enums.h"

namespace banggame {

    using visit_cards = play_visitor<target_type::cards_other_players>;

    template<> game_string visit_cards::get_error(const effect_context &ctx, const serial::card_list &target_cards) {
        if (!rn::all_of(origin->m_game->m_players | rv::filter(&player::alive), [&](player *p) {
            size_t found = rn::count(target_cards, p, &card::owner);
            if (p->only_black_cards_equipped()) return found == 0;
            if (p == origin || p == ctx.skipped_player) return found == 0;
            else return found == 1;
        })) {
            return "ERROR_INVALID_TARGETS";
        } else {
            for (card *c : target_cards) {
                if (c->deck == card_deck_type::character) {
                    return "ERROR_TARGET_NOT_CARD";
                }
                MAYBE_RETURN(effect.get_error(origin_card, origin, c, ctx));
            }
            return {};
        }
    }

    template<> game_string visit_cards::prompt(const effect_context &ctx, const serial::card_list &target_cards) {
        if (target_cards.empty()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        game_string msg;
        for (card *target_card : target_cards) {
            msg = effect.on_prompt(origin_card, origin, target_card, ctx);
            if (!msg) break;
        }
        return msg;
    }

    template<> void visit_cards::add_context(effect_context &ctx, const serial::card_list &target_cards) {
        for (card *target_card : target_cards) {
            effect.add_context(origin_card, origin, ctx);
        }
    }

    template<> void visit_cards::play(const effect_context &ctx, const serial::card_list &target_cards) {
        auto flags = effect_flags::multi_target;
        if (origin_card->is_brown()) {
            flags |= effect_flags::escapable;
        }
        if (target_cards.size() == 1) {
            flags |= effect_flags::single_target;
        }
        for (card *target_card : target_cards) {
            if (target_card->pocket == pocket_type::player_hand) {
                effect.on_play(origin_card, origin, target_card->owner->random_hand_card(), flags, ctx);
            } else {
                effect.on_play(origin_card, origin, target_card, flags, ctx);
            }
        }
    }

}