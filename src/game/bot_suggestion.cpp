#include "bot_suggestion.h"
#include "game_table.h"

#include "cards/game_enums.h"
#include "cards/filter_enums.h"

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
            if (target_card->pocket == pocket_type::player_table && target_card->has_tag(tag_type::penalty)) {
                signal_helpful_action(origin, owner, flags);
            } else {
                signal_hostile_action(origin, owner, flags);
            }
        }
    }

    bool is_target_enemy(const_player_ptr origin, const_player_ptr target, action_type type) {
        if (origin == target) return false;
        if (origin->m_game->check_flags(game_flag::free_for_all)) return true;

        switch (origin->m_role) {
        case player_role::outlaw:
        case player_role::shadow_outlaw:
            if (is_role_known(origin, target)) {
                return target->is_sheriff_or_deputy();
            } else {
                return is_positive_karma(target);
            }
        case player_role::sheriff:
            if (is_role_known(origin, target)) {
                return target->is_outlaw_or_renegade();
            } else if (type == action_type::damage && rn::none_of(origin->m_game->m_players, [](player_ptr p) { return p->in_game() && p->is_outlaw(); })) {
                // when all targets are deputy or renegade the sheriff will damage everyone until they are 1 hp
                // "neutral" actions will target neutral or negative players, as per the default
                return target->m_hp > 1;
            } else if (is_negative_karma(target)) {
                return true;
            } else if (is_positive_karma(target)) {
                return false;
            } else if (type == action_type::damage) {
                // the sheriff won't kill neutral targets
                return target->m_hp > 1;
            } else {
                // don't equip jail on neutral targets
                return type != action_type::jail;
            }
        case player_role::deputy:
        case player_role::shadow_deputy:
            if (is_role_known(origin, target)) {
                return target->is_outlaw_or_renegade();
            } else if (is_negative_karma(target)) {
                return true;
            } else if (is_positive_karma(target)) {
                return false;
            } else {
                return type != action_type::jail;
            }
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
                if (origin == target) {
                    return false;
                } else if (is_role_known(origin, target)) {
                    return target->is_outlaw_or_renegade();
                } else if (is_negative_karma(target)) {
                    return true;
                } else if (is_positive_karma(target)) {
                    return false;
                } else {
                    return type != action_type::jail;
                }
            } else if (num_law > 1) {
                if (is_role_known(origin, target)) {
                    return target->is_deputy();
                } else if (is_negative_karma(target)) {
                    return false;
                } else if (is_positive_karma(target)) {
                    return true;
                } else {
                    return type != action_type::jail;
                }
            } else if (target->is_sheriff() && num_bandit > 0) {
                // the renegade will not damage the sheriff if he has <= 2 hp
                // but will still target him with "neutral" actions
                return type != action_type::damage || target->m_hp > 2;
            } else {
                return true;
            }
        }
        case player_role::outlaw_3p:
            return origin->m_game->num_alive(true) <= 2 || target->m_role == player_role::deputy_3p;
        case player_role::deputy_3p:
            return origin->m_game->num_alive(true) <= 2 || target->m_role == player_role::renegade_3p;
        case player_role::renegade_3p:
            return origin->m_game->num_alive(true) <= 2 || target->m_role == player_role::outlaw_3p;
        default:
            return true;
        }
    }

    bool is_target_friend(const_player_ptr origin, const_player_ptr target) {
        if (origin == target) return true;
        if (origin->m_game->check_flags(game_flag::free_for_all)) return false;

        switch (origin->m_role) {
        case player_role::outlaw:
        case player_role::shadow_outlaw:
            if (is_role_known(origin, target)) {
                return target->is_outlaw();
            } else {
                return is_negative_karma(target);
            }
        case player_role::sheriff:
        case player_role::deputy:
        case player_role::shadow_deputy:
            if (is_role_known(origin, target)) {
                return target->is_sheriff_or_deputy();
            } else {
                return is_positive_karma(target);
            }
        case player_role::renegade:
        default:
            return false;
        }
    }
}