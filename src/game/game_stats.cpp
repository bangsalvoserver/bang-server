#include "game_stats.h"

#include "game_table.h"

#include "game_options.h"

#include "cards/bang_cards.h"
#include "cards/filter_enums.h"
#include "cards/game_enums.h"
#include "cards/game_events.h"

#include "effects/base/requests.h"
#include "effects/base/death.h"
#include "effects/base/dynamite.h"
#include "effects/base/duel.h"
#include "effects/base/jail.h"
#include "effects/base/damage.h"
#include "effects/base/heal.h"
#include "effects/base/draw_check.h"

#include <chrono>

namespace banggame {

    static int64_t unix_now() {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    void game_tracking::init_tracking(game_ptr game) {
        m_started_at = unix_now();

        game->add_listener<event_type::on_play_card>(nullptr, [this](player_ptr origin, card_ptr origin_card, const effect_context &ctx) {
            if (origin_card->is_bang_card(origin)) {
                ++m_stats[origin].bangs_played;
                if (int &turn_count = m_turn_bang_count[origin]; ++turn_count > 1) {
                    ++m_stats[origin].volcanic_bangs_played;
                }
            } else if (origin_card->pocket == pocket_type::player_character) {
                ++m_stats[origin].ability_uses;
            }
        });

        game->add_listener<event_type::on_turn_start>(nullptr, [this](player_ptr origin) {
            m_turn_bang_count[origin] = 0;
        });

        game->add_listener<event_type::on_turn_switch>(nullptr, [this, game](player_ptr origin) {
            if (origin == game->m_first_player) {
                ++m_rounds;
            }
        });

        game->add_listener<event_type::on_player_eliminated>(nullptr, [this](player_ptr killer, player_ptr target, death_type type) {
            if (type == death_type::death && killer && killer != target) {
                ++m_stats[killer].kills;
            }
            m_elimination_order[target] = m_next_elimination_order++;
            m_died_on_round[target] = m_rounds;
        });

        game->add_listener<event_type::on_dynamite_explode>(nullptr, [this](player_ptr target) {
            ++m_stats[target].dynamite_explosions;
        });

        game->add_listener<event_type::on_duel_lost>(nullptr, [this](player_ptr target) {
            ++m_stats[target].duels_lost;
        });

        game->add_listener<event_type::on_jail_turn_skipped>(nullptr, [this](player_ptr target) {
            ++m_stats[target].prison_turns_skipped;
        });

        game->add_listener<event_type::on_card_added_to_hand>(nullptr, [this](player_ptr origin, card_ptr target_card) {
            ++m_stats[origin].cards_drawn;
        });

        game->add_listener<event_type::on_extra_cards_drawn>(nullptr, [this](player_ptr origin, card_ptr origin_card, int ncards) {
            ++m_stats[origin].bonus_draws_used;
        });

        game->add_listener<event_type::on_hit>(nullptr, [this](card_ptr origin_card, player_ptr origin, player_ptr target, int damage, effect_flags flags) {
            if (origin && origin != target) {
                m_stats[origin].damage_dealt += damage;
            }
        });

        game->add_listener<event_type::on_heal>(nullptr, [this](card_ptr origin_card, player_ptr origin, player_ptr target, int amount) {
            m_stats[target].hp_recovered += amount;
        });

        game->add_listener<event_type::on_draw_check_luck>(nullptr, [this](player_ptr target, bool lucky) {
            ++m_stats[target].draw_checks_total;
            if (lucky) {
                ++m_stats[target].draw_checks_lucky;
            }
        });
    }

    game_report game_tracking::get_game_report(const_game_ptr game) const {
        game_report report;
        report.started_at = m_started_at;
        report.ended_at = unix_now();
        report.num_rounds = m_rounds;
        report.num_players = static_cast<int>(game->m_players.size());
        report.options = game->m_options;

        for (player_ptr p : game->m_players) {
            player_game_report &entry = report.players.emplace_back();
            entry.user_id = p->user_id;
            entry.is_bot = p->is_bot();
            entry.role = p->get_base_role();
            entry.survived = p->in_game() && p->alive();
            entry.won = p->check_player_flags(player_flag::winner);
            if (card_ptr character = p->get_character()) {
                entry.character = std::string(character->name);
            }
            if (auto it = m_stats.find(p); it != m_stats.end()) {
                entry.stats = it->second;
            }
            if (auto it = m_elimination_order.find(p); it != m_elimination_order.end()) {
                entry.elimination_order = it->second;
            }
            if (auto it = m_died_on_round.find(p); it != m_died_on_round.end()) {
                entry.died_on_round = it->second;
            }
        }

        return report;
    }

}