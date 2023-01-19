#include "bot_suggestion.h"
#include "game.h"

namespace banggame::bot_suggestion {

    bool target_enemy::on_check_target(card *origin_card, player *origin, player *target) {
        switch (origin->m_role) {
        case player_role::outlaw:
            return target->m_role == player_role::sheriff
                || target->m_role == player_role::deputy;
        case player_role::sheriff:
        case player_role::deputy:
            return target->m_role == player_role::outlaw
                || target->m_role == player_role::renegade;
        case player_role::renegade: {
            auto alive_players = origin->m_game->m_players | std::views::filter(&player::alive);
            auto num_outlaws = std::ranges::count(alive_players, player_role::outlaw, &player::m_role);
            auto num_sheriff_or_deputy = std::ranges::count_if(alive_players, [](player_role role) {
                return role == player_role::sheriff
                    || role == player_role::deputy;
            }, &player::m_role);
            if (num_outlaws == 0 && num_sheriff_or_deputy == 0) {
                if (std::ranges::distance(alive_players) == 1) {
                    return target->m_role == player_role::sheriff;
                } else {
                    return target->m_role == player_role::renegade;
                }
            } else if (num_outlaws > num_sheriff_or_deputy) {
                return target->m_role == player_role::outlaw;
            } else if (num_outlaws < num_sheriff_or_deputy) {
                return target->m_role == player_role::deputy;
            } else {
                return true;
            }
        }
        case player_role::outlaw_3p:
            return target->m_role == player_role::deputy_3p;
        case player_role::deputy_3p:
            return target->m_role == player_role::renegade_3p;
        case player_role::renegade_3p:
            return target->m_role == player_role::outlaw_3p;
        default:
            return true;
        }
    }
    
    bool target_enemy::on_check_target(card *origin_card, player *origin, card *target) {
        return on_check_target(origin_card, origin, target->owner);
    }

    bool target_friend::on_check_target(card *origin_card, player *origin, player *target) {
        switch (origin->m_role) {
        case player_role::outlaw:
            return target->m_role == player_role::outlaw;
        case player_role::sheriff:
        case player_role::deputy:
            return target->m_role == player_role::sheriff
                || target->m_role == player_role::deputy;
        case player_role::renegade:
        default:
            return origin == target;
        }
    }
    
    bool target_friend::on_check_target(card *origin_card, player *origin, card *target) {
        return on_check_target(origin_card, origin, target->owner);
    }
}