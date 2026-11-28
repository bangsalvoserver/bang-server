#include "bot_suggestion.h"
#include "game_table.h"

#include "cards/game_enums.h"
#include "cards/filter_enums.h"

#include "effects/ghost_cards/ruleset.h"
#include "effects/frontier/ruleset.h"

namespace banggame::bot_suggestion {

    static bool is_positive_karma(const_player_ptr origin) {
        return origin->check_player_flags(player_flag::positive_karma);
    }

    static bool is_negative_karma(const_player_ptr origin) {
        return origin->check_player_flags(player_flag::negative_karma);
    }

    static void set_karma_positive(player_ptr origin) {
        origin->remove_player_flags(player_flag::negative_karma);
        origin->add_player_flags(player_flag::positive_karma);
    }

    static void set_karma_negative(player_ptr origin) {
        origin->remove_player_flags(player_flag::positive_karma);
        origin->add_player_flags(player_flag::negative_karma);
    }

    static void set_karma_neutral(player_ptr origin) {
        origin->remove_player_flags(player_flag::positive_karma);
        origin->remove_player_flags(player_flag::negative_karma);
    }

    static bool is_role_known(const_player_ptr origin, const_player_ptr target) {
        if (!target->check_player_flags(player_flag::role_revealed)) {
            player_role first_role = player_role::unknown;
            for (player_ptr p : origin->m_game->m_players) {
                if (p != origin && !p->check_player_flags(player_flag::role_revealed)) {
                    player_role role = p->m_role;
                    if (first_role == player_role::unknown) {
                        first_role = role;
                    } else if (first_role != role) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    void signal_hostile_action(player_ptr origin, const_player_ptr target, effect_flags flags) {
        if (origin == target || origin->check_player_flags(player_flag::role_revealed)) return;

        if (flags.check(effect_flag::target_players)) return;

        // if (flags.check(effect_flag::multi_target)) return;
        // This is ignored but could make it so the ordering of attacks matter, which is probably fine

        if (target->check_player_flags(player_flag::role_revealed)) {
            switch (target->m_role) {
            case player_role::sheriff:
            case player_role::deputy:
            case player_role::shadow_deputy:
                set_karma_negative(origin);
                break;
            case player_role::outlaw:
            case player_role::shadow_outlaw:
            case player_role::renegade:
                set_karma_positive(origin);
                break;
            default:
                // ignore
                break;
            }
        } else {
            //     N   0   P
            //   +---+---+---+
            // N | 0 | 0 | N |
            //   +---+---+---+
            // 0 | P | P | N |
            //   +---+---+---+
            // P | P | P | 0 |
            //   +---+---+---+
            if (is_positive_karma(target)) {
                if (is_positive_karma(origin)) {
                    set_karma_neutral(origin);
                } else {
                    set_karma_negative(origin);
                }
            } else if (is_negative_karma(origin)) {
                set_karma_neutral(origin);
            } else {
                set_karma_positive(origin);
            }
        }
    }

    void signal_helpful_action(player_ptr origin, const_player_ptr target, effect_flags flags) {
        if (origin == target || origin->check_player_flags(player_flag::role_revealed)) return;

        if (flags.check(effect_flag::target_players)) return;

        if (target->check_player_flags(player_flag::role_revealed)) {
            switch (target->m_role) {
            case player_role::sheriff:
            case player_role::deputy:
            case player_role::shadow_deputy:
                if (is_negative_karma(origin)) {
                    set_karma_neutral(origin);
                } else {
                    set_karma_positive(origin);
                }
                break;
            case player_role::outlaw:
            case player_role::shadow_outlaw:
                if (is_positive_karma(origin)) {
                    set_karma_neutral(origin);
                } else {
                    set_karma_negative(origin);
                }
                break;
            case player_role::renegade:
                set_karma_neutral(origin);
                break;
            default:
                // ignore
                break;
            }
        } else {
            //     N   0   P
            //   +---+---+---+
            // N | N | 0 | 0 |
            //   +---+---+---+
            // 0 | N | 0 | P |
            //   +---+---+---+
            // P | 0 | 0 | P |
            //   +---+---+---+
            if (is_negative_karma(target) && !is_positive_karma(origin)) {
                set_karma_negative(origin);
            } else if (is_positive_karma(target) && !is_negative_karma(origin)) {
                set_karma_positive(origin);
            } else {
                set_karma_neutral(origin);
            }
        }
    }

    void signal_remove_card(player_ptr origin, const_card_ptr target_card, effect_flags flags) {
        if (const_player_ptr owner = target_card->owner) {
            if (auto value = target_card->get_tag_value(tag_type::penalty); value && target_card->pocket == pocket_type::player_table) {
                if (*value >= 0) signal_helpful_action(origin, owner, flags);
            } else {
                signal_hostile_action(origin, owner, flags);
            }
        }
    }

    player_alignment get_alignment(const_player_ptr origin, const_player_ptr target) {
        if (origin == target) return player_alignment::friendly;
        if (origin->m_game->check_flags(game_flag::free_for_all)) return player_alignment::enemy;

        switch (origin->m_role) {
        case player_role::outlaw:
        case player_role::shadow_outlaw:
            if (is_role_known(origin, target)) {
                if (target->is_sheriff_or_deputy()) {
                    return player_alignment::enemy;
                } else if (target->is_outlaw()) {
                    return player_alignment::friendly;
                } else if (target->is_renegade()) {
                    return player_alignment::maybe_enemy;
                }
            } else if (is_positive_karma(target)) {
                return player_alignment::enemy;
            } else if (is_negative_karma(target)) {
                return player_alignment::friendly;
            }
            return player_alignment::neutral;
        case player_role::sheriff:
        case player_role::deputy:
        case player_role::shadow_deputy:
            if (is_role_known(origin, target)) {
                if (target->is_outlaw_or_renegade()) {
                    return player_alignment::enemy;
                } else if (target->is_sheriff_or_deputy()) {
                    return player_alignment::friendly;
                }
            } else if (!origin->is_sheriff() || rn::any_of(origin->m_game->m_players, [](player_ptr p) { return p->in_game() && p->is_outlaw(); })) {
                if (is_negative_karma(target)) {
                    return player_alignment::enemy;
                } else if (is_positive_karma(target)) {
                    return player_alignment::friendly;
                }
            }
            return player_alignment::maybe_enemy;
        case player_role::renegade: {
            int num_law = 0;
            int num_bandit = 0;

            for (player_ptr p : origin->m_game->m_players) {
                if (p != origin && p->in_game()) {
                    if (p->is_sheriff_or_deputy()) ++num_law;
                    if (p->is_outlaw_or_renegade()) ++num_bandit;
                }
            }

            if (num_bandit > num_law) {
                if (is_role_known(origin, target)) {
                    if (target->is_outlaw_or_renegade()) {
                        return player_alignment::enemy;
                    } else if (target->is_sheriff_or_deputy()) {
                        return player_alignment::neutral;
                    }
                } else if (is_negative_karma(target)) {
                    return player_alignment::enemy;
                } else if (is_positive_karma(target)) {
                    return player_alignment::neutral;
                }
                return player_alignment::maybe_enemy;
            } else if (num_law > 1) {
                if (is_role_known(origin, target)) {
                    if (target->is_deputy()) {
                        return player_alignment::enemy;
                    } else {
                        return player_alignment::neutral;
                    }
                } else if (is_negative_karma(target)) {
                    return player_alignment::neutral;
                } else if (is_positive_karma(target)) {
                    return player_alignment::enemy;
                }
                return player_alignment::maybe_enemy;
            } else if (target->is_sheriff() && num_bandit > 0 && target->m_hp <= 2) {
                return player_alignment::neutral;
            }
            return player_alignment::enemy;
        }
        case player_role::outlaw_3p:
            if (origin->m_game->num_alive(true) <= 2 || target->m_role == player_role::deputy_3p) {
                return player_alignment::enemy;
            } else {
                return player_alignment::neutral;
            }
        case player_role::deputy_3p:
            if (origin->m_game->num_alive(true) <= 2 || target->m_role == player_role::renegade_3p) {
                return player_alignment::enemy;
            } else {
                return player_alignment::neutral;
            }
        case player_role::renegade_3p:
            if (origin->m_game->num_alive(true) <= 2 || target->m_role == player_role::outlaw_3p) {
                return player_alignment::enemy;
            } else {
                return player_alignment::neutral;
            }
        default:
            return player_alignment::neutral;
        }
    }

    int evaluate_attack(const_player_ptr origin, const_player_ptr target) {
        // TODO: should prioritize attacking weaker targets better
        const bool lethal = target->m_hp <= 1;

        if (target->is_sheriff()) {
            if (origin->is_outlaw()) {
                if (target->m_hp <= 2) return 10;
            } else if (lethal && (!origin->is_renegade() || origin->m_game->num_alive(true) > 2)) {
                return -50;
            }
        }

        switch (get_alignment(origin, target)) {
        case player_alignment::enemy:
            return lethal ? 8 : 7;
        case player_alignment::maybe_enemy:
            if (lethal) {
                return origin->is_sheriff() ? -5 : 6;
            } else {
                return 5;
            }
        case player_alignment::friendly:
            if (lethal) {
                return origin->is_sheriff() ? -20 : -10;
            } else {
                return -3;
            }
        case player_alignment::neutral:
        default:
            return -2;
        }
    }

    int evaluate_soft_attack(const_player_ptr origin, const_player_ptr target) {
        switch (get_alignment(origin, target)) {
        case player_alignment::enemy:
        case player_alignment::maybe_enemy:
            return 4;
        case player_alignment::friendly:
            return -5;
        case player_alignment::neutral:
        default:
            return -1;
        }
    }

    int evaluate_help(const_player_ptr origin, const_player_ptr target) {
        switch (get_alignment(origin, target)) {
        case player_alignment::enemy:
        case player_alignment::maybe_enemy:
            return -5;
        case player_alignment::friendly:
            return 4;
        case player_alignment::neutral:
        default:
            return -2;
        }
    }

    int evaluate_target_card(const_player_ptr origin, const_card_ptr target_card) {
        player_ptr target = target_card->owner;
        if (target->is_ghost()) {
            bool ghost_card = target_card->pocket == pocket_type::player_table && is_ghost_card(target_card);
            switch (get_alignment(origin, target)) {
            case player_alignment::enemy:
            case player_alignment::maybe_enemy:
                return ghost_card ? 20 : -10;
            case player_alignment::friendly:
                return ghost_card ? -10 : -20;
            case player_alignment::neutral:
            default:
                return -5;
            }
        }

        if (target_card->pocket == pocket_type::player_table && target_card->is_purple()) {
            if (player_ptr tracked_player = get_tracked_player(target_card)) {
                return target_card->has_tag(tag_type::pardner_penalty)
                    ? evaluate_help(origin, tracked_player)
                    : evaluate_soft_attack(origin, tracked_player);
            }
            return 0;
        }

        switch (get_alignment(origin, target)) {
        case player_alignment::enemy:
        case player_alignment::maybe_enemy:
            if (target_card->pocket == pocket_type::player_table) {
                if (auto value = target_card->get_tag_value(tag_type::penalty)) {
                    return *value >= 0 ? -5 : 0;
                } else if (auto value = target_card->get_tag_value(tag_type::strong)) {
                    return 8 + *value;
                }
            }
            return 5;
        case player_alignment::friendly:
            if (target_card->pocket == pocket_type::player_table) {
                if (auto value = target_card->get_tag_value(tag_type::penalty)) {
                    if (*value >= 0) {
                        return 6;
                    } else if (origin != target) {
                        return 0;
                    }
                } else if (auto value = target_card->get_tag_value(tag_type::strong)) {
                    return -7 - *value;
                }
            }
            return -6;
        case player_alignment::neutral:
        default:
            return -1;
        }
    }

    int evaluate_draw_card(const_player_ptr origin, const_card_ptr target_card) {
        if (auto value = target_card->get_tag_value(tag_type::strong)) {
            return 4 + *value;
        }

        if (target_card->has_tag(tag_type::jail)) {
            return 1;
        }

        if (origin->is_ghost()) {
            return 0;
        }

        if (origin->m_game->num_alive() <= 2 && target_card->has_tag(tag_type::beer)) {
            return -2;
        }

        if (origin->m_hp < origin->m_max_hp && (target_card->has_tag(tag_type::healing) || target_card->has_tag(tag_type::missedlike))) {
            return 2;
        }

        return 0;
    }

    int evaluate_discard_card(const_player_ptr origin, const_card_ptr target_card) {
        if (auto value = target_card->get_tag_value(tag_type::strong)) {
            return -2 - *value;
        }

        if (target_card->pocket == pocket_type::player_table && target_card->has_tag(tag_type::penalty)) {
            return 5;
        }

        if (origin->is_ghost()) {
            return 0;
        }

        if (origin->m_game->num_alive() <= 2 && target_card->has_tag(tag_type::beer)) {
            return 4;
        }

        if (origin->m_hp < origin->m_max_hp && target_card->has_tag(tag_type::healing)) {
            return -2;
        }

        if (origin->m_hp <= 2 && target_card->has_tag(tag_type::missedlike)) {
            return -3;
        }

        return 0;
    }
}
