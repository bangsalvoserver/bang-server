#include "play_visitor.h"

namespace banggame {

    using namespace enums::flag_operators;

    template<> opt_game_str play_visitor<target_type::none>::verify(const play_card_verify *verifier, const effect_holder &effect) {
        return effect.verify(verifier->card_ptr, verifier->origin);
    }

    template<> opt_game_str play_visitor<target_type::none>::prompt(const play_card_verify *verifier, const effect_holder &effect) {
        return effect.on_prompt(verifier->card_ptr, verifier->origin);
    }

    template<> void play_visitor<target_type::none>::play(const play_card_verify *verifier, const effect_holder &effect) {
        effect.on_play(verifier->card_ptr, verifier->origin, effect_flags{});
    }

    template<> opt_game_str play_visitor<target_type::player>::verify(const play_card_verify *verifier, const effect_holder &effect, player *target) {
        if (auto error = check_player_filter(verifier->card_ptr, verifier->origin, effect.player_filter, target)) {
            return error;
        } else {
            return effect.verify(verifier->card_ptr, verifier->origin, target);
        }
    }

    template<> opt_game_str play_visitor<target_type::player>::prompt(const play_card_verify *verifier, const effect_holder &effect, player *target) {
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

    template<> opt_game_str play_visitor<target_type::conditional_player>::verify(const play_card_verify *verifier, const effect_holder &effect, nullable<player> target) {
        if (target) {
            return play_visitor<target_type::player>{}.verify(verifier, effect, target);
        } else if (!verifier->origin->make_player_target_set(verifier->card_ptr, effect).empty()) {
            return game_string("ERROR_TARGET_SET_NOT_EMPTY");
        } else {
            return std::nullopt;
        }
    }

    template<> opt_game_str play_visitor<target_type::conditional_player>::prompt(const play_card_verify *verifier, const effect_holder &effect, nullable<player> target) {
        if (target) {
            return play_visitor<target_type::player>{}.prompt(verifier, effect, target);
        } else {
            return std::nullopt;
        }
    }

    template<> void play_visitor<target_type::conditional_player>::play(const play_card_verify *verifier, const effect_holder &effect, nullable<player> target) {
        if (target) {
            play_visitor<target_type::player>{}.play(verifier, effect, target);
        }
    }

    template<> opt_game_str play_visitor<target_type::other_players>::verify(const play_card_verify *verifier, const effect_holder &effect) {
        for (player &p : range_other_players(verifier->origin)) {
            if (auto error = effect.verify(verifier->card_ptr, verifier->origin, &p)) {
                return error;
            }
        }
        return std::nullopt;
    }

    template<> opt_game_str play_visitor<target_type::other_players>::prompt(const play_card_verify *verifier, const effect_holder &effect) {
        opt_game_str msg = std::nullopt;
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

    template<> opt_game_str play_visitor<target_type::all_players>::verify(const play_card_verify *verifier, const effect_holder &effect) {
        for (player &p : range_all_players(verifier->origin)) {
            if (auto error = effect.verify(verifier->card_ptr, verifier->origin, &p)) {
                return error;
            }
        }
        return std::nullopt;
    }

    template<> opt_game_str play_visitor<target_type::all_players>::prompt(const play_card_verify *verifier, const effect_holder &effect) {
        opt_game_str msg = std::nullopt;
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

    template<> opt_game_str play_visitor<target_type::card>::verify(const play_card_verify *verifier, const effect_holder &effect, card *target) {
        if (!target->owner) {
            return game_string("ERROR_CARD_HAS_NO_OWNER");
        } else if (auto error = check_player_filter(verifier->card_ptr, verifier->origin, effect.player_filter, target->owner)) {
            return error;
        } else if (auto error = check_card_filter(verifier->card_ptr, verifier->origin, effect.card_filter, target)) {
            return error;
        } else {
            return effect.verify(verifier->card_ptr, verifier->origin, target);
        }
    }

    template<> opt_game_str play_visitor<target_type::card>::prompt(const play_card_verify *verifier, const effect_holder &effect, card *target) {
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

    template<> opt_game_str play_visitor<target_type::extra_card>::verify(const play_card_verify *verifier, const effect_holder &effect, nullable<card> target) {
        if (!target) {
            if (verifier->card_ptr != verifier->origin->m_last_played_card) {
                return game_string("ERROR_TARGET_SET_NOT_EMPTY");
            } else {
                return std::nullopt;
            }
        } else if (target->owner != verifier->origin || target->pocket != pocket_type::player_hand || target == verifier->card_ptr) {
            return game_string("ERROR_TARGET_NOT_SELF");
        } else {
            return effect.verify(verifier->card_ptr, verifier->origin, target);
        }
    }

    template<> opt_game_str play_visitor<target_type::extra_card>::prompt(const play_card_verify *verifier, const effect_holder &effect, nullable<card> target) {
        if (target) {
            return effect.on_prompt(verifier->card_ptr, verifier->origin, target);
        } else {
            return std::nullopt;
        }
    }

    template<> void play_visitor<target_type::extra_card>::play(const play_card_verify *verifier, const effect_holder &effect, nullable<card> target) {
        if (target) {
            effect.on_play(verifier->card_ptr, verifier->origin, target, effect_flags{});
        }
    }

    template<> opt_game_str play_visitor<target_type::cards_other_players>::verify(const play_card_verify *verifier, const effect_holder &effect, const std::vector<card *> &target_cards) {
        if (!std::ranges::all_of(verifier->origin->m_game->m_players | std::views::filter(&player::alive), [&](const player &p) {
            size_t found = std::ranges::count(target_cards, &p, &card::owner);
            if (p.m_hand.empty() && p.m_table.empty()) return found == 0;
            if (&p == verifier->origin) return found == 0;
            else return found == 1;
        })) {
            return game_string("ERROR_INVALID_TARGETS");
        } else {
            for (card *c : target_cards) {
                if (auto error = effect.verify(verifier->card_ptr, verifier->origin, c)) {
                    return error;
                }
            }
            return std::nullopt;
        }
    }

    template<> opt_game_str play_visitor<target_type::cards_other_players>::prompt(const play_card_verify *verifier, const effect_holder &effect, const std::vector<card *> &target_cards) {
        opt_game_str msg = std::nullopt;
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

    template<> opt_game_str play_visitor<target_type::cube>::verify(const play_card_verify *verifier, const effect_holder &effect, const std::vector<card *> &target_cards) {
        for (card *c : target_cards) {
            if (!c || c->owner != verifier->origin) {
                return game_string("ERROR_TARGET_NOT_SELF");
            }
            if (auto error = effect.verify(verifier->card_ptr, verifier->origin, c)) {
                return error;
            }
        }
        return std::nullopt;
    }

    template<> opt_game_str play_visitor<target_type::cube>::prompt(const play_card_verify *verifier, const effect_holder &effect, const std::vector<card *> &target_cards) {
        opt_game_str msg = std::nullopt;
        for (card *target_card : target_cards) {
            msg = effect.on_prompt(verifier->card_ptr, verifier->origin, target_card);
            if (!msg) break;
        }
        return msg;
    }

    template<> void play_visitor<target_type::cube>::play(const play_card_verify *verifier, const effect_holder &effect, const std::vector<card *> &target_cards) {
        for (card *target_card : target_cards) {
            effect.on_play(verifier->card_ptr, verifier->origin, target_card, effect_flags{});
        }
    }
}