#include "play_visitor.h"
#include "possible_to_play.h"

namespace banggame {

    template<> game_string play_visitor<target_type::none>::verify(const effect_context &ctx) {
        return effect.verify(origin_card, origin, ctx);
    }

    template<> duplicate_set play_visitor<target_type::none>::duplicates() {
        return {};
    }

    template<> game_string play_visitor<target_type::none>::prompt(const effect_context &ctx) {
        return effect.on_prompt(origin_card, origin);
    }

    template<> void play_visitor<target_type::none>::play(const effect_context &ctx) {
        effect.on_play(origin_card, origin);
    }

    template<> game_string play_visitor<target_type::player>::verify(const effect_context &ctx, player *target) {
        MAYBE_RETURN(check_player_filter(origin, effect.player_filter, target, ctx));
        return effect.verify(origin_card, origin, target, ctx);
    }

    template<> duplicate_set play_visitor<target_type::player>::duplicates(player *target) {
        return player_set{target};
    }

    template<> game_string play_visitor<target_type::player>::prompt(const effect_context &ctx, player *target) {
        return effect.on_prompt(origin_card, origin, target);
    }

    template<> void play_visitor<target_type::player>::play(const effect_context &ctx, player *target) {
        auto flags = effect_flags::single_target;
        if (origin_card->is_brown()) {
            flags |= effect_flags::escapable;
        }
        effect.on_play(origin_card, origin, target, flags);
    }

    template<> game_string play_visitor<target_type::conditional_player>::verify(const effect_context &ctx, player *target) {
        if (target) {
            return play_visitor<target_type::player>{origin, origin_card, effect}.verify(ctx, target);
        } else if (contains_at_least(make_player_target_set(origin, origin_card, effect), 1)) {
            return "ERROR_TARGET_SET_NOT_EMPTY";
        } else {
            return {};
        }
    }

    template<> duplicate_set play_visitor<target_type::conditional_player>::duplicates(player *target) {
        if (target) {
            return player_set{target};
        } else {
            return {};
        }
    }

    template<> game_string play_visitor<target_type::conditional_player>::prompt(const effect_context &ctx, player *target) {
        if (target) {
            return play_visitor<target_type::player>{origin, origin_card, effect}.prompt(ctx, target);
        } else {
            return {};
        }
    }

    template<> void play_visitor<target_type::conditional_player>::play(const effect_context &ctx, player *target) {
        if (target) {
            play_visitor<target_type::player>{origin, origin_card, effect}.play(ctx, target);
        }
    }

    template<> game_string play_visitor<target_type::players>::verify(const effect_context &ctx) {
        for (player *target : range_all_players(origin)) {
            if (!check_player_filter(origin, effect.player_filter, target, ctx)) {
                MAYBE_RETURN(effect.verify(origin_card, origin, target, ctx));
            }
        }
        return {};
    }

    template<> duplicate_set play_visitor<target_type::players>::duplicates() {
        return {};
    }

    template<> game_string play_visitor<target_type::players>::prompt(const effect_context &ctx) {
        game_string msg;
        for (player *target : range_all_players(origin)) {
            if (!check_player_filter(origin, effect.player_filter, target, ctx)) {
                msg = effect.on_prompt(origin_card, origin, target);
                if (!msg) break;
            }
        }
        return msg;
    }

    template<> void play_visitor<target_type::players>::play(const effect_context &ctx) {
        std::vector<player *> targets;
        for (player *target : range_all_players(origin)) {
            if (!check_player_filter(origin, effect.player_filter, target, ctx)) {
                targets.push_back(target);
            }
        }

        auto flags = effect_flags::multi_target;
        if (targets.size() == 1) {
            flags |= effect_flags::single_target;
        }
        if (origin_card->is_brown()) {
            flags |= effect_flags::escapable;
        }

        for (player *target : targets) {
            effect.on_play(origin_card, origin, target, flags);
        }
    }

    template<> game_string play_visitor<target_type::fanning_targets>::verify(const effect_context &ctx, const std::vector<not_null<player *>> &targets) {
        if (targets.size() != 2) {
            return "ERROR_INVALID_TARGETS";
        }
        MAYBE_RETURN(check_player_filter(origin, target_player_filter::notself | target_player_filter::reachable, targets.front(), ctx));
        if (origin->m_game->calc_distance(targets.front(), targets.back()) > 1) {
            return "ERROR_TARGETS_NOT_ADJACENT";
        }
        return {};
    }

    template<> duplicate_set play_visitor<target_type::fanning_targets>::duplicates(const std::vector<not_null<player *>> &targets) {
        return player_set{targets.front(), targets.back()};
    }

    template<> game_string play_visitor<target_type::fanning_targets>::prompt(const effect_context &ctx, const std::vector<not_null<player *>> &targets) {
        for (player *target : targets) {
            MAYBE_RETURN(play_visitor<target_type::player>{origin, origin_card, effect}.prompt(ctx, target));
        }
        return {};
    }

    template<> void play_visitor<target_type::fanning_targets>::play(const effect_context &ctx, const std::vector<not_null<player *>> &targets) {
        auto flags = effect_flags{};
        if (origin_card->is_brown()) {
            flags |= effect_flags::escapable;
        }
        for (player *target : targets) {
            effect.on_play(origin_card, origin, target, flags);
        }
    }

    template<> game_string play_visitor<target_type::card>::verify(const effect_context &ctx, card *target) {
        if (!target->owner) return "ERROR_CARD_HAS_NO_OWNER";
        MAYBE_RETURN(check_player_filter(origin, effect.player_filter, target->owner, ctx));
        MAYBE_RETURN(check_card_filter(origin_card, origin, effect.card_filter, target, ctx));
        return effect.verify(origin_card, origin, target, ctx);
    }

    template<> duplicate_set play_visitor<target_type::card>::duplicates(card *target) {
        if (bool(effect.card_filter & target_card_filter::can_repeat)) {
            return {};
        } else {
            return card_set{target};
        }
    }

    template<> game_string play_visitor<target_type::card>::prompt(const effect_context &ctx, card *target) {
        return effect.on_prompt(origin_card, origin, target);
    }

    template<> void play_visitor<target_type::card>::play(const effect_context &ctx, card *target) {
        auto flags = effect_flags::single_target;
        if (origin_card->is_brown()) {
            flags |= effect_flags::escapable;
        }
        if (target->owner != origin && target->pocket == pocket_type::player_hand) {
            effect.on_play(origin_card, origin, target->owner->random_hand_card(), flags);
        } else {
            effect.on_play(origin_card, origin, target, flags);
        }
    }

    template<> game_string play_visitor<target_type::extra_card>::verify(const effect_context &ctx, card *target) {
        if (!target) {
            if (ctx.repeating) {
                return {};
            } else {
                return "ERROR_TARGET_SET_NOT_EMPTY";
            }
        } else {
            return play_visitor<target_type::card>{origin, origin_card, effect}.verify(ctx, target);
        }
    }

    template<> duplicate_set play_visitor<target_type::extra_card>::duplicates(card *target) {
        if (!target || bool(effect.card_filter & target_card_filter::can_repeat)) {
            return {};
        } else {
            return card_set{target};
        }
    }

    template<> game_string play_visitor<target_type::extra_card>::prompt(const effect_context &ctx, card *target) {
        if (target) {
            return effect.on_prompt(origin_card, origin, target);
        } else {
            return {};
        }
    }

    template<> void play_visitor<target_type::extra_card>::play(const effect_context &ctx, card *target) {
        if (target) {
            effect.on_play(origin_card, origin, target);
        }
    }

    template<> game_string play_visitor<target_type::cards>::verify(const effect_context &ctx, const std::vector<not_null<card *>> &targets) {
        if (targets.size() != std::max<size_t>(1, effect.target_value)) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card *c : targets) {
            MAYBE_RETURN(play_visitor<target_type::card>{origin, origin_card, effect}.verify(ctx, c));
        }
        return {};
    }

    template<> duplicate_set play_visitor<target_type::cards>::duplicates(const std::vector<not_null<card *>> &targets) {
        if (bool(effect.card_filter & target_card_filter::can_repeat)) {
            return {};
        } else {
            card_set ret;
            for (card *target : targets) {
                ret.emplace(target);
            }
            return ret;
        }
    }

    template<> game_string play_visitor<target_type::cards>::prompt(const effect_context &ctx, const std::vector<not_null<card *>> &targets) {
        for (card *c : targets) {
            MAYBE_RETURN(play_visitor<target_type::card>{origin, origin_card, effect}.prompt(ctx, c));
        }
        return {};
    }

    template<> void play_visitor<target_type::cards>::play(const effect_context &ctx, const std::vector<not_null<card *>> &targets) {
        for (card *c : targets) {
            play_visitor<target_type::card>{origin, origin_card, effect}.play(ctx, c);
        }
    }

    template<> game_string play_visitor<target_type::cards_other_players>::verify(const effect_context &ctx, const std::vector<not_null<card *>> &target_cards) {
        if (!std::ranges::all_of(origin->m_game->m_players | std::views::filter(&player::alive), [&](player *p) {
            size_t found = std::ranges::count(target_cards, p, &card::owner);
            if (p->only_black_cards_equipped()) return found == 0;
            if (p == origin) return found == 0;
            else return found == 1;
        })) {
            return "ERROR_INVALID_TARGETS";
        } else {
            for (card *c : target_cards) {
                MAYBE_RETURN(effect.verify(origin_card, origin, c, ctx));
            }
            return {};
        }
    }

    template<> duplicate_set play_visitor<target_type::cards_other_players>::duplicates(const std::vector<not_null<card *>> &target_cards) {
        return {};
    }

    template<> game_string play_visitor<target_type::cards_other_players>::prompt(const effect_context &ctx, const std::vector<not_null<card *>> &target_cards) {
        if (target_cards.empty()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        game_string msg;
        for (card *target_card : target_cards) {
            msg = effect.on_prompt(origin_card, origin, target_card);
            if (!msg) break;
        }
        return msg;
    }

    template<> void play_visitor<target_type::cards_other_players>::play(const effect_context &ctx, const std::vector<not_null<card *>> &target_cards) {
        auto flags = effect_flags::multi_target;
        if (origin_card->is_brown()) {
            flags |= effect_flags::escapable;
        }
        if (target_cards.size() == 1) {
            flags |= effect_flags::single_target;
        }
        for (card *target_card : target_cards) {
            if (target_card->pocket == pocket_type::player_hand) {
                effect.on_play(origin_card, origin, target_card->owner->random_hand_card(), flags);
            } else {
                effect.on_play(origin_card, origin, target_card, flags);
            }
        }
    }

    template<> game_string play_visitor<target_type::select_cubes>::verify(const effect_context &ctx, const std::vector<not_null<card *>> &target_cards) {
        if (effect.type != effect_type::pay_cube) {
            return "ERROR_INVALID_EFFECT_TYPE";
        }
        if (target_cards.size() != effect.target_value) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card *c : target_cards) {
            if (!c || c->owner != origin) {
                return "ERROR_TARGET_NOT_SELF";
            }
        }
        return {};
    }

    template<> duplicate_set play_visitor<target_type::select_cubes>::duplicates(const std::vector<not_null<card *>> &target_cards) {
        card_cube_count ret;
        for (card *target : target_cards) {
            ++ret[target];
        }
        return ret;
    }

    template<> game_string play_visitor<target_type::select_cubes>::prompt(const effect_context &ctx, const std::vector<not_null<card *>> &target_cards) {
        return {};
    }

    template<> void play_visitor<target_type::select_cubes>::play(const effect_context &ctx, const std::vector<not_null<card *>> &target_cards) {}

    template<> game_string play_visitor<target_type::self_cubes>::verify(const effect_context &ctx) {
        if (effect.type != effect_type::pay_cube) {
            return "ERROR_INVALID_EFFECT_TYPE";
        }
        return {};
    }

    template<> duplicate_set play_visitor<target_type::self_cubes>::duplicates() {
        return card_cube_count{
            {origin_card, effect.target_value}
        };
    }

    template<> game_string play_visitor<target_type::self_cubes>::prompt(const effect_context &ctx) {
        return {};
    }

    template<> void play_visitor<target_type::self_cubes>::play(const effect_context &ctx) {}
}