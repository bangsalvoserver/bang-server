#include "bot_suggestion.h"
#include "game_table.h"

#include "cards/game_enums.h"

namespace banggame::bot_suggestion {

    static bool is_positive_karma(const_player_ptr origin) {
        return origin->check_player_flags(player_flag::positive_karma);
    }

    static void set_karma_positive(player_ptr origin) {
        origin->remove_player_flags(player_flag::negative_karma);
        origin->add_player_flags(player_flag::positive_karma);
    }

    static bool is_negative_karma(const_player_ptr origin) {
        return origin->check_player_flags(player_flag::negative_karma);
    }

    static void set_karma_negative(player_ptr origin) {
        origin->remove_player_flags(player_flag::positive_karma);
        origin->add_player_flags(player_flag::negative_karma);
    }

    static bool is_neutral_karma(const_player_ptr origin) {
        return !is_positive_karma(origin) && !is_negative_karma(origin);
    }

    static void set_karma_neutral(player_ptr origin) {
        origin->remove_player_flags(player_flag::positive_karma);
        origin->remove_player_flags(player_flag::negative_karma);
    }

    static bool is_role_visible(const_player_ptr origin, const_player_ptr target) {
        return origin == target || target->check_player_flags(player_flag::role_revealed);
    }

    static bool is_same_karma(const_player_ptr origin, const_player_ptr target) {
        return (is_positive_karma(origin) && is_positive_karma(target))
            || (is_negative_karma(origin) && is_negative_karma(target));
    }

    void signal_hostile_action(player_ptr origin, const_player_ptr target, effect_flags flags, const effect_context &ctx) {
        if (origin == target) return;

        // TODO improve these?
        if (flags.check(effect_flag::target_players)) return;
        if (flags.check(effect_flag::multi_target)) return;

        if (is_role_visible(origin, target)) {
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
        } else if (is_same_karma(origin, target)) {
            set_karma_neutral(origin);
        } else if (is_negative_karma(target) || is_neutral_karma(target)) {
            set_karma_positive(origin);
        } else if (is_positive_karma(target)) {
            set_karma_negative(origin);
        }
    }

    void signal_helpful_action(player_ptr origin, const_player_ptr target, effect_flags flags, const effect_context &ctx) {
        if (origin == target) return;

        // TODO improve these?
        if (flags.check(effect_flag::target_players)) return;
        if (flags.check(effect_flag::multi_target)) return;

        if (is_role_visible(origin, target)) {
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
        } else if (is_same_karma(origin, target)) {
            // ignore, origin is helping a teammate
        } else if (is_negative_karma(target)) {
            set_karma_negative(origin);
        } else if (is_positive_karma(target)) {
            set_karma_positive(origin);
        } else {
            // origin helping player with neutral karma
            set_karma_neutral(origin);
        }
    }

    bool is_target_enemy(const_player_ptr origin, const_player_ptr target) {
        if (origin->m_game->check_flags(game_flag::free_for_all)) {
            return origin != target;
        }
        switch (origin->m_role) {
        case player_role::outlaw:
        case player_role::shadow_outlaw:
            if (is_role_visible(origin, target)) {
                return target->m_role == player_role::sheriff
                    || target->m_role == player_role::deputy
                    || target->m_role == player_role::shadow_deputy;
            } else {
                return is_positive_karma(target);
            }
        case player_role::sheriff:
        case player_role::deputy:
        case player_role::shadow_deputy:
            if (is_role_visible(origin, target)) {
                return target->m_role == player_role::outlaw
                    || target->m_role == player_role::renegade
                    || target->m_role == player_role::shadow_outlaw;
            } else {
                return is_negative_karma(target) || is_neutral_karma(target);
            }
        case player_role::renegade: {
            auto targets = origin->m_game->m_players | rv::filter([origin](player_ptr p) {
                return p != origin && p->alive();
            });
            auto num_outlaws = rn::count_if(targets, [](player_role role) {
                return role == player_role::outlaw
                    || role == player_role::renegade
                    || role == player_role::shadow_outlaw;
            }, &player::m_role);
            auto num_sheriff_or_deputy = rn::count_if(targets, [](player_role role) {
                return role == player_role::sheriff
                    || role == player_role::deputy
                    || role == player_role::shadow_deputy;
            }, &player::m_role);
            if (num_outlaws > num_sheriff_or_deputy) {
                if (origin == target) {
                    return false;
                } else if (is_role_visible(origin, target)) {
                    return target->m_role == player_role::outlaw
                        || target->m_role == player_role::shadow_outlaw
                        || target->m_role == player_role::renegade;
                } else {
                    return is_negative_karma(target) || is_neutral_karma(target);
                }
            } else if (num_sheriff_or_deputy > 1) {
                if (is_role_visible(origin, target)) {
                    return target->m_role == player_role::deputy
                        || target->m_role == player_role::shadow_deputy;
                } else {
                    return is_positive_karma(target) || is_neutral_karma(target);
                }
            } else if (target->m_role == player_role::sheriff && num_outlaws > 0) {
                return target->m_hp > 2;
            } else {
                return true;
            }
        }
        case player_role::outlaw_3p:
            return origin->m_game->num_alive() <= 2 || target->m_role == player_role::deputy_3p;
        case player_role::deputy_3p:
            return origin->m_game->num_alive() <= 2 || target->m_role == player_role::renegade_3p;
        case player_role::renegade_3p:
            return origin->m_game->num_alive() <= 2 || target->m_role == player_role::outlaw_3p;
        default:
            return true;
        }
    }

    bool is_target_friend(const_player_ptr origin, const_player_ptr target) {
        if (origin->m_game->check_flags(game_flag::free_for_all)) {
            return origin == target;
        }
        switch (origin->m_role) {
        case player_role::outlaw:
        case player_role::shadow_outlaw:
            if (is_role_visible(origin, target)) {
                return target->m_role == player_role::outlaw
                    || target->m_role == player_role::shadow_outlaw;
            } else {
                return is_negative_karma(target);
            }
        case player_role::sheriff:
        case player_role::deputy:
        case player_role::shadow_deputy:
            if (is_role_visible(origin, target)) {
                return target->m_role == player_role::sheriff
                    || target->m_role == player_role::deputy
                    || target->m_role == player_role::shadow_deputy;
            } else {
                return is_positive_karma(target);
            }
        case player_role::renegade:
        default:
            return origin == target;
        }
    }
}