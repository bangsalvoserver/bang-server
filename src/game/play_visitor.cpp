#include "play_visitor.h"

namespace banggame {

    using namespace enums::flag_operators;

    template<> game_string play_visitor<target_type::none>::verify(const play_card_verify *verifier, const effect_holder &effect) {
        return effect.verify(verifier->card_ptr, verifier->origin);
    }

    template<> game_string play_visitor<target_type::none>::prompt(const play_card_verify *verifier, const effect_holder &effect) {
        return effect.on_prompt(verifier->card_ptr, verifier->origin);
    }

    template<> void play_visitor<target_type::none>::play(const play_card_verify *verifier, const effect_holder &effect) {
        effect.on_play(verifier->card_ptr, verifier->origin, effect_flags{});
    }

    template<> game_string play_visitor<target_type::player>::verify(const play_card_verify *verifier, const effect_holder &effect, player *target) {
        if (game_string error = check_player_filter(verifier->card_ptr, verifier->origin, effect.player_filter, target)) {
            return error;
        } else {
            return effect.verify(verifier->card_ptr, verifier->origin, target);
        }
    }

    template<> game_string play_visitor<target_type::player>::prompt(const play_card_verify *verifier, const effect_holder &effect, player *target) {
        return effect.on_prompt(verifier->card_ptr, verifier->origin, target);
    }

    template<> void play_visitor<target_type::player>::play(const play_card_verify *verifier, const effect_holder &effect, player *target) {
        auto flags = effect_flags::single_target;
        if (verifier->card_ptr->color == card_color_type::brown) {
            flags |= effect_flags::escapable;
        }
        if (!target->immune_to(verifier->card_ptr, verifier->origin, flags)) {
            effect.on_play(verifier->card_ptr, verifier->origin, target, flags);
        }
    }

    template<> game_string play_visitor<target_type::conditional_player>::verify(const play_card_verify *verifier, const effect_holder &effect, nullable<player> target) {
        if (target) {
            return play_visitor<target_type::player>{}.verify(verifier, effect, target);
        } else if (!verifier->origin->make_player_target_set(verifier->card_ptr, effect).empty()) {
            return "ERROR_TARGET_SET_NOT_EMPTY";
        } else {
            return {};
        }
    }

    template<> game_string play_visitor<target_type::conditional_player>::prompt(const play_card_verify *verifier, const effect_holder &effect, nullable<player> target) {
        if (target) {
            return play_visitor<target_type::player>{}.prompt(verifier, effect, target);
        } else {
            return {};
        }
    }

    template<> void play_visitor<target_type::conditional_player>::play(const play_card_verify *verifier, const effect_holder &effect, nullable<player> target) {
        if (target) {
            play_visitor<target_type::player>{}.play(verifier, effect, target);
        }
    }

    template<> game_string play_visitor<target_type::other_players>::verify(const play_card_verify *verifier, const effect_holder &effect) {
        for (player &p : range_other_players(verifier->origin)) {
            if (game_string error = effect.verify(verifier->card_ptr, verifier->origin, &p)) {
                return error;
            }
        }
        return {};
    }

    template<> game_string play_visitor<target_type::other_players>::prompt(const play_card_verify *verifier, const effect_holder &effect) {
        game_string msg;
        for (player &p : range_other_players(verifier->origin)) {
            msg = effect.on_prompt(verifier->card_ptr, verifier->origin, &p);
            if (!msg) break;
        }
        return msg;
    }

    template<> void play_visitor<target_type::other_players>::play(const play_card_verify *verifier, const effect_holder &effect) {
        auto targets = range_other_players(verifier->origin);
        
        effect_flags flags{};
        if (verifier->card_ptr->color == card_color_type::brown) {
            flags |= effect_flags::escapable;
        }
        if (std::ranges::distance(targets) == 1) {
            flags |= effect_flags::single_target;
        }
        for (player &p : targets) {
            if (!p.immune_to(verifier->card_ptr, verifier->origin, flags)) {
                effect.on_play(verifier->card_ptr, verifier->origin, &p, flags);
            }
        }
    }

    template<> game_string play_visitor<target_type::all_players>::verify(const play_card_verify *verifier, const effect_holder &effect) {
        for (player &p : range_all_players(verifier->origin)) {
            if (game_string error = effect.verify(verifier->card_ptr, verifier->origin, &p)) {
                return error;
            }
        }
        return {};
    }

    template<> game_string play_visitor<target_type::all_players>::prompt(const play_card_verify *verifier, const effect_holder &effect) {
        game_string msg;
        for (player &p : range_all_players(verifier->origin)) {
            msg = effect.on_prompt(verifier->card_ptr, verifier->origin, &p);
            if (!msg) break;
        }
        return msg;
    }

    template<> void play_visitor<target_type::all_players>::play(const play_card_verify *verifier, const effect_holder &effect) {
        effect_flags flags{};
        if (verifier->card_ptr->color == card_color_type::brown) {
            flags |= effect_flags::escapable;
        }
        for (player &p : range_all_players(verifier->origin)) {
            if (!p.immune_to(verifier->card_ptr, verifier->origin, flags)) {
                effect.on_play(verifier->card_ptr, verifier->origin, &p, flags);
            }
        }
    }

    template<> game_string play_visitor<target_type::card>::verify(const play_card_verify *verifier, const effect_holder &effect, card *target) {
        if (!target->owner) {
            return "ERROR_CARD_HAS_NO_OWNER";
        } else if (game_string error = check_player_filter(verifier->card_ptr, verifier->origin, effect.player_filter, target->owner)) {
            return error;
        } else if (game_string error = check_card_filter(verifier->card_ptr, verifier->origin, effect.card_filter, target)) {
            return error;
        } else {
            return effect.verify(verifier->card_ptr, verifier->origin, target);
        }
    }

    template<> game_string play_visitor<target_type::card>::prompt(const play_card_verify *verifier, const effect_holder &effect, card *target) {
        return effect.on_prompt(verifier->card_ptr, verifier->origin, target);
    }

    template<> void play_visitor<target_type::card>::play(const play_card_verify *verifier, const effect_holder &effect, card *target) {
        auto flags = effect_flags::single_target;
        if (verifier->card_ptr->color == card_color_type::brown) {
            flags |= effect_flags::escapable;
        }
        if (target->owner == verifier->origin) {
            effect.on_play(verifier->card_ptr, verifier->origin, target, flags);
        } else if (!target->owner->immune_to(verifier->card_ptr, verifier->origin, flags)) {
            if (target->pocket == pocket_type::player_hand) {
                effect.on_play(verifier->card_ptr, verifier->origin, target->owner->random_hand_card(), flags);
            } else {
                effect.on_play(verifier->card_ptr, verifier->origin, target, flags);
            }
        }
    }

    template<> game_string play_visitor<target_type::extra_card>::verify(const play_card_verify *verifier, const effect_holder &effect, nullable<card> target) {
        if (!target) {
            if (verifier->card_ptr != verifier->origin->m_last_played_card) {
                return "ERROR_TARGET_SET_NOT_EMPTY";
            } else {
                return {};
            }
        } else if (target->owner != verifier->origin || target->pocket != pocket_type::player_hand || target == verifier->card_ptr) {
            return "ERROR_TARGET_NOT_SELF";
        } else {
            return effect.verify(verifier->card_ptr, verifier->origin, target);
        }
    }

    template<> game_string play_visitor<target_type::extra_card>::prompt(const play_card_verify *verifier, const effect_holder &effect, nullable<card> target) {
        if (target) {
            return effect.on_prompt(verifier->card_ptr, verifier->origin, target);
        } else {
            return {};
        }
    }

    template<> void play_visitor<target_type::extra_card>::play(const play_card_verify *verifier, const effect_holder &effect, nullable<card> target) {
        if (target) {
            effect.on_play(verifier->card_ptr, verifier->origin, target, effect_flags{});
        }
    }

    template<> game_string play_visitor<target_type::cards>::verify(const play_card_verify *verifier, const effect_holder &effect, const std::vector<card *> &targets) {
        if (targets.size() != std::max<size_t>(1, effect.target_value)) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card *c : targets) {
            if (game_string err = play_visitor<target_type::card>{}.verify(verifier, effect, c)) {
                return err;
            }
        }
        return {};
    }

    template<> game_string play_visitor<target_type::cards>::prompt(const play_card_verify *verifier, const effect_holder &effect, const std::vector<card *> &targets) {
        for (card *c : targets) {
            if (game_string err = play_visitor<target_type::card>{}.prompt(verifier, effect, c)) {
                return err;
            }
        }
        return {};
    }

    template<> void play_visitor<target_type::cards>::play(const play_card_verify *verifier, const effect_holder &effect, const std::vector<card *> &targets) {
        for (card *c : targets) {
            play_visitor<target_type::card>{}.play(verifier, effect, c);
        }
    }

    template<> game_string play_visitor<target_type::cards_other_players>::verify(const play_card_verify *verifier, const effect_holder &effect, const std::vector<card *> &target_cards) {
        if (!std::ranges::all_of(verifier->origin->m_game->m_players | std::views::filter(&player::alive), [&](const player &p) {
            size_t found = std::ranges::count(target_cards, &p, &card::owner);
            if (p.m_hand.empty() && p.m_table.empty()) return found == 0;
            if (&p == verifier->origin) return found == 0;
            else return found == 1;
        })) {
            return "ERROR_INVALID_TARGETS";
        } else {
            for (card *c : target_cards) {
                if (game_string error = effect.verify(verifier->card_ptr, verifier->origin, c)) {
                    return error;
                }
            }
            return {};
        }
    }

    template<> game_string play_visitor<target_type::cards_other_players>::prompt(const play_card_verify *verifier, const effect_holder &effect, const std::vector<card *> &target_cards) {
        game_string msg;
        for (card *target_card : target_cards) {
            msg = effect.on_prompt(verifier->card_ptr, verifier->origin, target_card);
            if (!msg) break;
        }
        return msg;
    }

    template<> void play_visitor<target_type::cards_other_players>::play(const play_card_verify *verifier, const effect_holder &effect, const std::vector<card *> &target_cards) {
        effect_flags flags{};
        if (verifier->card_ptr->color == card_color_type::brown) {
            flags |= effect_flags::escapable;
        }
        if (target_cards.size() == 1) {
            flags |= effect_flags::single_target;
        }
        for (card *target_card : target_cards) {
            if (target_card->pocket == pocket_type::player_hand) {
                effect.on_play(verifier->card_ptr, verifier->origin, target_card->owner->random_hand_card(), flags);
            } else {
                effect.on_play(verifier->card_ptr, verifier->origin, target_card, flags);
            }
        }
    }

    template<> game_string play_visitor<target_type::select_cubes>::verify(const play_card_verify *verifier, const effect_holder &effect, const std::vector<card *> &target_cards) {
        if (effect.type != effect_type::pay_cube) {
            return "ERROR_INVALID_EFFECT_TYPE";
        }
        for (card *c : target_cards) {
            if (!c || c->owner != verifier->origin) {
                return "ERROR_TARGET_NOT_SELF";
            }
        }
        return {};
    }

    template<> game_string play_visitor<target_type::select_cubes>::prompt(const play_card_verify *verifier, const effect_holder &effect, const std::vector<card *> &target_cards) {
        return {};
    }

    template<> void play_visitor<target_type::select_cubes>::play(const play_card_verify *verifier, const effect_holder &effect, const std::vector<card *> &target_cards) {}

    template<> game_string play_visitor<target_type::self_cubes>::verify(const play_card_verify *verifier, const effect_holder &effect, int ncubes) {
        if (effect.type != effect_type::pay_cube) {
            return "ERROR_INVALID_EFFECT_TYPE";
        } else if (ncubes != effect.target_value) {
            return "ERROR_INVALID_NCUBES";
        }
        return {};
    }

    template<> game_string play_visitor<target_type::self_cubes>::prompt(const play_card_verify *verifier, const effect_holder &effect, int ncubes) {
        return {};
    }

    template<> void play_visitor<target_type::self_cubes>::play(const play_card_verify *verifier, const effect_holder &effect, int ncubes) {}
}