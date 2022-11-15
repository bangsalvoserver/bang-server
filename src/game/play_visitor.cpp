#include "play_visitor.h"

namespace banggame {

    template<> game_string play_visitor<target_type::none>::verify() {
        return effect.verify(verifier.origin_card, verifier.origin);
    }

    template<> duplicate_set play_visitor<target_type::none>::duplicates() {
        return {};
    }

    template<> game_string play_visitor<target_type::none>::prompt() {
        return effect.on_prompt(verifier.origin_card, verifier.origin);
    }

    template<> void play_visitor<target_type::none>::play() {
        effect.on_play(verifier.origin_card, verifier.origin);
    }

    template<> game_string play_visitor<target_type::player>::verify(player *target) {
        if (auto error = check_player_filter(verifier.origin, effect.player_filter, target)) {
            return error;
        } else {
            return effect.verify(verifier.origin_card, verifier.origin, target);
        }
    }

    template<> duplicate_set play_visitor<target_type::player>::duplicates(player *target) {
        return player_set{target};
    }

    template<> game_string play_visitor<target_type::player>::prompt(player *target) {
        return effect.on_prompt(verifier.origin_card, verifier.origin, target);
    }

    template<> void play_visitor<target_type::player>::play(player *target) {
        auto flags = effect_flags::single_target;
        if (verifier.origin_card->color == card_color_type::brown) {
            flags |= effect_flags::escapable;
        }
        if (!target->immune_to(verifier.origin_card, verifier.origin, flags)) {
            effect.on_play(verifier.origin_card, verifier.origin, target, flags);
        }
    }

    template<> game_string play_visitor<target_type::conditional_player>::verify(player *target) {
        if (target) {
            return play_visitor<target_type::player>{verifier, effect}.verify(target);
        } else if (!verifier.origin->make_player_target_set(verifier.origin_card, effect).empty()) {
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

    template<> game_string play_visitor<target_type::conditional_player>::prompt(player *target) {
        if (target) {
            return play_visitor<target_type::player>{verifier, effect}.prompt(target);
        } else {
            return {};
        }
    }

    template<> void play_visitor<target_type::conditional_player>::play(player *target) {
        if (target) {
            play_visitor<target_type::player>{verifier, effect}.play(target);
        }
    }

    template<> game_string play_visitor<target_type::other_players>::verify() {
        for (player &p : range_other_players(verifier.origin)) {
            if (game_string error = effect.verify(verifier.origin_card, verifier.origin, &p)) {
                return error;
            }
        }
        return {};
    }

    template<> duplicate_set play_visitor<target_type::other_players>::duplicates() {
        return {};
    }

    template<> game_string play_visitor<target_type::other_players>::prompt() {
        game_string msg;
        for (player &p : range_other_players(verifier.origin)) {
            msg = effect.on_prompt(verifier.origin_card, verifier.origin, &p);
            if (!msg) break;
        }
        return msg;
    }

    template<> void play_visitor<target_type::other_players>::play() {
        auto targets = range_other_players(verifier.origin);
        
        auto flags = effect_flags::multi_target;
        if (verifier.origin_card->color == card_color_type::brown) {
            flags |= effect_flags::escapable;
        }
        if (std::ranges::distance(targets) == 1) {
            flags |= effect_flags::single_target;
        }
        for (player &p : targets) {
            if (!p.immune_to(verifier.origin_card, verifier.origin, flags)) {
                effect.on_play(verifier.origin_card, verifier.origin, &p, flags);
            }
        }
    }

    template<> game_string play_visitor<target_type::all_players>::verify() {
        for (player &p : range_all_players(verifier.origin)) {
            if (game_string error = effect.verify(verifier.origin_card, verifier.origin, &p)) {
                return error;
            }
        }
        return {};
    }

    template<> duplicate_set play_visitor<target_type::all_players>::duplicates() {
        return {};
    }

    template<> game_string play_visitor<target_type::all_players>::prompt() {
        game_string msg;
        for (player &p : range_all_players(verifier.origin)) {
            msg = effect.on_prompt(verifier.origin_card, verifier.origin, &p);
            if (!msg) break;
        }
        return msg;
    }

    template<> void play_visitor<target_type::all_players>::play() {
        auto flags = effect_flags::multi_target;
        if (verifier.origin_card->color == card_color_type::brown) {
            flags |= effect_flags::escapable;
        }
        for (player &p : range_all_players(verifier.origin)) {
            if (!p.immune_to(verifier.origin_card, verifier.origin, flags)) {
                effect.on_play(verifier.origin_card, verifier.origin, &p, flags);
            }
        }
    }

    template<> game_string play_visitor<target_type::card>::verify(card *target) {
        if (!target->owner) {
            return "ERROR_CARD_HAS_NO_OWNER";
        } else if (auto error = check_player_filter(verifier.origin, effect.player_filter, target->owner)) {
            return error;
        } else if (auto error = check_card_filter(verifier.origin_card, verifier.origin, effect.card_filter, target)) {
            return error;
        } else {
            return effect.verify(verifier.origin_card, verifier.origin, target);
        }
    }

    template<> duplicate_set play_visitor<target_type::card>::duplicates(card *target) {
        if (bool(effect.card_filter & target_card_filter::can_repeat)) {
            return {};
        } else {
            return card_set{target};
        }
    }

    template<> game_string play_visitor<target_type::card>::prompt(card *target) {
        return effect.on_prompt(verifier.origin_card, verifier.origin, target);
    }

    template<> void play_visitor<target_type::card>::play(card *target) {
        auto flags = effect_flags::single_target;
        if (verifier.origin_card->color == card_color_type::brown) {
            flags |= effect_flags::escapable;
        }
        if (target->owner == verifier.origin) {
            effect.on_play(verifier.origin_card, verifier.origin, target, flags);
        } else if (!target->owner->immune_to(verifier.origin_card, verifier.origin, flags)) {
            if (target->pocket == pocket_type::player_hand) {
                effect.on_play(verifier.origin_card, verifier.origin, target->owner->random_hand_card(), flags);
            } else {
                effect.on_play(verifier.origin_card, verifier.origin, target, flags);
            }
        }
    }

    template<> game_string play_visitor<target_type::extra_card>::verify(card *target) {
        if (!target) {
            if (verifier.origin_card != verifier.origin->m_last_played_card) {
                return "ERROR_TARGET_SET_NOT_EMPTY";
            } else {
                return {};
            }
        } else {
            return play_visitor<target_type::card>{verifier, effect}.verify(target);
        }
    }

    template<> duplicate_set play_visitor<target_type::extra_card>::duplicates(card *target) {
        if (!target || bool(effect.card_filter & target_card_filter::can_repeat)) {
            return {};
        } else {
            return card_set{target};
        }
    }

    template<> game_string play_visitor<target_type::extra_card>::prompt(card *target) {
        if (target) {
            return effect.on_prompt(verifier.origin_card, verifier.origin, target);
        } else {
            return {};
        }
    }

    template<> void play_visitor<target_type::extra_card>::play(card *target) {
        if (target) {
            effect.on_play(verifier.origin_card, verifier.origin, target);
        }
    }

    template<> game_string play_visitor<target_type::cards>::verify(const std::vector<not_null<card *>> &targets) {
        if (targets.size() != std::max<size_t>(1, effect.target_value)) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card *c : targets) {
            if (game_string err = play_visitor<target_type::card>{verifier, effect}.verify(c)) {
                return err;
            }
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

    template<> game_string play_visitor<target_type::cards>::prompt(const std::vector<not_null<card *>> &targets) {
        for (card *c : targets) {
            if (game_string err = play_visitor<target_type::card>{verifier, effect}.prompt(c)) {
                return err;
            }
        }
        return {};
    }

    template<> void play_visitor<target_type::cards>::play(const std::vector<not_null<card *>> &targets) {
        for (card *c : targets) {
            play_visitor<target_type::card>{verifier, effect}.play(c);
        }
    }

    template<> game_string play_visitor<target_type::cards_other_players>::verify(const std::vector<not_null<card *>> &target_cards) {
        if (!std::ranges::all_of(verifier.origin->m_game->m_players | std::views::filter(&player::alive), [&](const player &p) {
            size_t found = std::ranges::count(target_cards, &p, &card::owner);
            if (p.m_hand.empty() && p.m_table.empty()) return found == 0;
            if (&p == verifier.origin) return found == 0;
            else return found == 1;
        })) {
            return "ERROR_INVALID_TARGETS";
        } else {
            for (card *c : target_cards) {
                if (game_string error = effect.verify(verifier.origin_card, verifier.origin, c)) {
                    return error;
                }
            }
            return {};
        }
    }

    template<> duplicate_set play_visitor<target_type::cards_other_players>::duplicates(const std::vector<not_null<card *>> &target_cards) {
        return {};
    }

    template<> game_string play_visitor<target_type::cards_other_players>::prompt(const std::vector<not_null<card *>> &target_cards) {
        if (target_cards.empty()) {
            return {"PROMPT_CARD_NO_EFFECT", verifier.origin_card};
        }
        game_string msg;
        for (card *target_card : target_cards) {
            msg = effect.on_prompt(verifier.origin_card, verifier.origin, target_card);
            if (!msg) break;
        }
        return msg;
    }

    template<> void play_visitor<target_type::cards_other_players>::play(const std::vector<not_null<card *>> &target_cards) {
        auto flags = effect_flags::multi_target;
        if (verifier.origin_card->color == card_color_type::brown) {
            flags |= effect_flags::escapable;
        }
        if (target_cards.size() == 1) {
            flags |= effect_flags::single_target;
        }
        for (card *target_card : target_cards) {
            if (target_card->pocket == pocket_type::player_hand) {
                effect.on_play(verifier.origin_card, verifier.origin, target_card->owner->random_hand_card(), flags);
            } else {
                effect.on_play(verifier.origin_card, verifier.origin, target_card, flags);
            }
        }
    }

    template<> game_string play_visitor<target_type::select_cubes>::verify(const std::vector<not_null<card *>> &target_cards) {
        if (effect.type != effect_type::pay_cube) {
            return "ERROR_INVALID_EFFECT_TYPE";
        }
        for (card *c : target_cards) {
            if (!c || c->owner != verifier.origin) {
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

    template<> game_string play_visitor<target_type::select_cubes>::prompt(const std::vector<not_null<card *>> &target_cards) {
        return {};
    }

    template<> void play_visitor<target_type::select_cubes>::play(const std::vector<not_null<card *>> &target_cards) {}

    template<> game_string play_visitor<target_type::self_cubes>::verify() {
        if (effect.type != effect_type::pay_cube) {
            return "ERROR_INVALID_EFFECT_TYPE";
        }
        return {};
    }

    template<> duplicate_set play_visitor<target_type::self_cubes>::duplicates() {
        return card_cube_count{
            {verifier.origin_card, effect.target_value}
        };
    }

    template<> game_string play_visitor<target_type::self_cubes>::prompt() {
        return {};
    }

    template<> void play_visitor<target_type::self_cubes>::play() {}
}