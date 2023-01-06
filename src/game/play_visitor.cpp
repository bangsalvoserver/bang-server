#include "play_visitor.h"
#include "possible_to_play.h"

namespace banggame {

    template<> game_string play_visitor<target_type::none>::verify() {
        return effect.verify(verifier.get_playing_card(), verifier.origin);
    }

    template<> duplicate_set play_visitor<target_type::none>::duplicates() {
        return {};
    }

    template<> game_string play_visitor<target_type::none>::prompt() {
        return effect.on_prompt(verifier.get_playing_card(), verifier.origin);
    }

    template<> void play_visitor<target_type::none>::play() {
        effect.on_play(verifier.get_playing_card(), verifier.origin);
    }

    template<> game_string play_visitor<target_type::player>::verify(player *target) {
        if (auto error = check_player_filter(verifier.origin, effect.player_filter, target)) {
            return error;
        } else {
            return effect.verify(verifier.get_playing_card(), verifier.origin, target);
        }
    }

    template<> duplicate_set play_visitor<target_type::player>::duplicates(player *target) {
        return player_set{target};
    }

    template<> game_string play_visitor<target_type::player>::prompt(player *target) {
        return effect.on_prompt(verifier.get_playing_card(), verifier.origin, target);
    }

    template<> void play_visitor<target_type::player>::play(player *target) {
        card *playing_card = verifier.get_playing_card();
        auto flags = effect_flags::single_target;
        if (playing_card->is_brown()) {
            flags |= effect_flags::escapable;
        }
        effect.on_play(playing_card, verifier.origin, target, flags);
    }

    template<> game_string play_visitor<target_type::conditional_player>::verify(player *target) {
        if (target) {
            return play_visitor<target_type::player>{verifier, effect}.verify(target);
        } else if (!make_player_target_set(verifier.origin, verifier.get_playing_card(), effect).empty()) {
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

    template<> game_string play_visitor<target_type::players>::verify() {
        card *playing_card = verifier.get_playing_card();
        for (player *target : range_all_players(verifier.origin)) {
            if (!check_player_filter(verifier.origin, effect.player_filter, target)) {
                if (game_string error = effect.verify(playing_card, verifier.origin, target)) {
                    return error;
                }
            }
        }
        return {};
    }

    template<> duplicate_set play_visitor<target_type::players>::duplicates() {
        return {};
    }

    template<> game_string play_visitor<target_type::players>::prompt() {
        card *playing_card = verifier.get_playing_card();
        game_string msg;
        for (player *target : range_all_players(verifier.origin)) {
            if (!check_player_filter(verifier.origin, effect.player_filter, target)) {
                msg = effect.on_prompt(playing_card, verifier.origin, target);
                if (!msg) break;
            }
        }
        return msg;
    }

    template<> void play_visitor<target_type::players>::play() {
        card *playing_card = verifier.get_playing_card();
        std::vector<player *> targets;
        for (player *target : range_all_players(verifier.origin)) {
            if (!check_player_filter(verifier.origin, effect.player_filter, target)) {
                targets.push_back(target);
            }
        }

        auto flags = effect_flags::multi_target;
        if (targets.size() == 1) {
            flags |= effect_flags::single_target;
        }
        if (playing_card->is_brown()) {
            flags |= effect_flags::escapable;
        }

        for (player *target : targets) {
            effect.on_play(playing_card, verifier.origin, target, flags);
        }
    }

    template<> game_string play_visitor<target_type::fanning_targets>::verify(const std::vector<not_null<player *>> &targets) {
        if (targets.size() != 2) {
            return "ERROR_INVALID_TARGETS";
        }
        card *playing_card = verifier.get_playing_card();
        if (auto error = check_player_filter(verifier.origin, target_player_filter::notself | target_player_filter::reachable, targets.front())) {
            return error;
        } else if (verifier.origin->m_game->calc_distance(targets.front(), targets.back()) > 1) {
            return "ERROR_TARGETS_NOT_ADJACENT";
        }
        return {};
    }

    template<> duplicate_set play_visitor<target_type::fanning_targets>::duplicates(const std::vector<not_null<player *>> &targets) {
        return player_set{targets.front(), targets.back()};
    }

    template<> game_string play_visitor<target_type::fanning_targets>::prompt(const std::vector<not_null<player *>> &targets) {
        for (player *target : targets) {
            if (game_string err = play_visitor<target_type::player>{verifier, effect}.prompt(target)) {
                return err;
            }
        }
        return {};
    }

    template<> void play_visitor<target_type::fanning_targets>::play(const std::vector<not_null<player *>> &targets) {
        card *playing_card = verifier.get_playing_card();
        auto flags = effect_flags{};
        if (playing_card->is_brown()) {
            flags |= effect_flags::escapable;
        }
        for (player *target : targets) {
            effect.on_play(playing_card, verifier.origin, target, flags);
        }
    }

    template<> game_string play_visitor<target_type::card>::verify(card *target) {
        card *playing_card = verifier.get_playing_card();
        if (!target->owner) {
            return "ERROR_CARD_HAS_NO_OWNER";
        } else if (auto error = check_player_filter(verifier.origin, effect.player_filter, target->owner)) {
            return error;
        } else if (auto error = check_card_filter(playing_card, verifier.origin, effect.card_filter, target)) {
            return error;
        } else {
            return effect.verify(playing_card, verifier.origin, target);
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
        return effect.on_prompt(verifier.get_playing_card(), verifier.origin, target);
    }

    template<> void play_visitor<target_type::card>::play(card *target) {
        card *playing_card = verifier.get_playing_card();
        auto flags = effect_flags::single_target;
        if (playing_card->is_brown()) {
            flags |= effect_flags::escapable;
        }
        if (target->owner != verifier.origin && target->pocket == pocket_type::player_hand) {
            effect.on_play(playing_card, verifier.origin, target->owner->random_hand_card(), flags);
        } else {
            effect.on_play(playing_card, verifier.origin, target, flags);
        }
    }

    template<> game_string play_visitor<target_type::extra_card>::verify(card *target) {
        if (!target) {
            if (verifier.get_playing_card() != verifier.origin->get_last_played_card().first) {
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
            return effect.on_prompt(verifier.get_playing_card(), verifier.origin, target);
        } else {
            return {};
        }
    }

    template<> void play_visitor<target_type::extra_card>::play(card *target) {
        if (target) {
            effect.on_play(verifier.get_playing_card(), verifier.origin, target);
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
        if (!std::ranges::all_of(verifier.origin->m_game->m_players | std::views::filter(&player::alive), [&](player *p) {
            size_t found = std::ranges::count(target_cards, p, &card::owner);
            if (p->only_black_cards_equipped()) return found == 0;
            if (p == verifier.origin) return found == 0;
            else return found == 1;
        })) {
            return "ERROR_INVALID_TARGETS";
        } else {
            card *playing_card = verifier.get_playing_card();
            for (card *c : target_cards) {
                if (game_string error = effect.verify(playing_card, verifier.origin, c)) {
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
        card *playing_card = verifier.get_playing_card();
        if (target_cards.empty()) {
            return {"PROMPT_CARD_NO_EFFECT", playing_card};
        }
        game_string msg;
        for (card *target_card : target_cards) {
            msg = effect.on_prompt(playing_card, verifier.origin, target_card);
            if (!msg) break;
        }
        return msg;
    }

    template<> void play_visitor<target_type::cards_other_players>::play(const std::vector<not_null<card *>> &target_cards) {
        card *playing_card = verifier.get_playing_card();
        auto flags = effect_flags::multi_target;
        if (playing_card->is_brown()) {
            flags |= effect_flags::escapable;
        }
        if (target_cards.size() == 1) {
            flags |= effect_flags::single_target;
        }
        for (card *target_card : target_cards) {
            if (target_card->pocket == pocket_type::player_hand) {
                effect.on_play(playing_card, verifier.origin, target_card->owner->random_hand_card(), flags);
            } else {
                effect.on_play(playing_card, verifier.origin, target_card, flags);
            }
        }
    }

    template<> game_string play_visitor<target_type::select_cubes>::verify(const std::vector<not_null<card *>> &target_cards) {
        if (effect.type != effect_type::pay_cube) {
            return "ERROR_INVALID_EFFECT_TYPE";
        }
        if (target_cards.size() != effect.target_value) {
            return "ERROR_INVALID_TARGETS";
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
            {verifier.get_playing_card(), effect.target_value}
        };
    }

    template<> game_string play_visitor<target_type::self_cubes>::prompt() {
        return {};
    }

    template<> void play_visitor<target_type::self_cubes>::play() {}
}