#include "death.h"

#include "requests.h"

#include "game/game_table.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    bool effect_deathsave::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_death>(target_is{origin}) != nullptr;
    }

    void effect_deathsave::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->top_request<request_death>()->tried_save = true;
    }

    void request_death::on_update() {
        if (target->m_hp <= 0 && !target->check_player_flags(player_flag::dead)) {
            auto_resolve();
        } else {
            target->m_game->pop_request();
        }
    }
    
    void request_death::on_resolve() {
        target->m_game->pop_request();
        target->m_game->call_event(event_type::on_player_death{ target, tried_save });

        handle_player_death(origin, target, true);
    }

    prompt_string request_death::resolve_prompt() const {
        return "PROMPT_DEATH";
    }

    game_string request_death::status_text(player_ptr owner) const {
        int nbeers = 1 - target->m_hp;
        if (target == owner) {
            return {"STATUS_DEATH", nbeers};
        } else {
            return {"STATUS_DEATH_OTHER", target, nbeers};
        }
    }
    
    void handle_player_death(player_ptr killer, player_ptr target, bool is_death) {
        if (killer != target->m_game->m_playing) killer = nullptr;
        
        target->m_game->queue_action([=]{
            if (target->m_hp <= 0) {
                if (killer && killer != target) {
                    target->m_game->add_log("LOG_PLAYER_KILLED", killer, target);
                } else {
                    target->m_game->add_log("LOG_PLAYER_DIED", target);
                }

                target->add_player_flags(player_flag::dead);
                target->set_hp(0, true);
            }

            if (!target->alive()) {
                target->remove_extra_characters();
                for (card_ptr c : target->m_characters) {
                    target->disable_equip(c);
                }

                if (target->add_player_flags(player_flag::role_revealed)) {
                    target->m_game->add_update<"player_show_role">(update_target::excludes(target), target, target->m_role);
                }

                target->m_game->call_event(event_type::on_player_eliminated{ killer, target });
            }
        }, 50);

        if (killer && is_death) {
            target->m_game->queue_action([=] {
                if (killer->alive() && !target->alive()) {
                    if (target->m_game->m_players.size() > 3) {
                        if (target->m_role == player_role::outlaw) {
                            target->m_game->add_log("LOG_KILLED_OUTLAW", killer);
                            killer->draw_card(3);
                        } else if (target->m_role == player_role::deputy && killer->m_role == player_role::sheriff) {
                            target->m_game->add_log("LOG_SHERIFF_KILLED_DEPUTY", killer);
                            target->m_game->queue_request<request_sheriff_killed_deputy>(killer, -4);
                        }
                    } else if (target->m_game->m_players.size() == 3 && (
                        (target->m_role == player_role::deputy_3p && killer->m_role == player_role::renegade_3p) ||
                        (target->m_role == player_role::outlaw_3p && killer->m_role == player_role::deputy_3p) ||
                        (target->m_role == player_role::renegade_3p && killer->m_role == player_role::outlaw_3p)))
                    {
                        killer->draw_card(3);
                    }
                }
            }, 50);
        }
        
        target->m_game->queue_action([=]{
            if (!target->alive()) {
                if (is_death) {
                    target->m_game->queue_request<request_discard_all_death>(target);
                } else {
                    target->m_game->queue_request<request_discard_all>(target);
                }
            }
        }, 50);

        if (rn::none_of(target->m_game->get_all_cards(), [](const_card_ptr c) { return c->has_tag(tag_type::ghost_card); })) {
            target->m_game->queue_action([=]{
                bool any_player_removed = false;
                for (player_ptr p : target->m_game->m_players) {
                    if (!p->alive() && p->add_player_flags(player_flag::removed)) {
                        any_player_removed = true;
                    }
                }
                
                if (any_player_removed) {
                    target->m_game->add_update<"player_order">(target->m_game->m_players);
                }
            }, -6);
        }

        target->m_game->queue_action([=] {
            if (target == target->m_game->m_first_player && !target->alive() && target->m_game->num_alive() > 1) {
                target->m_game->m_first_player = target->get_next_player();
            }

            auto declare_winners = [&](auto &&winners) {
                for (player_ptr p : target->m_game->range_all_players(target->m_game->m_playing)) {
                    if (p->add_player_flags(player_flag::role_revealed)) {
                        target->m_game->add_update<"player_show_role">(update_target::excludes(p), p, p->m_role);
                    }
                }
                target->m_game->add_log("LOG_GAME_OVER");
                for (player_ptr p : winners) {
                    p->add_player_flags(player_flag::winner);
                }
                target->m_game->add_game_flags(game_flag::game_over);
            };

            auto alive_players = rv::filter(target->m_game->m_players, &player::alive);

            if (target->m_game->check_flags(game_flag::free_for_all)) {
                if (rn::distance(alive_players) <= 1) {
                    declare_winners(alive_players);
                }
            } else if (target->m_game->m_players.size() > 3) {
                auto is_outlaw = [](player_ptr p) { return p->m_role == player_role::outlaw; };
                auto is_renegade = [](player_ptr p) { return p->m_role == player_role::renegade; };
                auto is_sheriff = [](player_ptr p) { return p->m_role == player_role::sheriff; };
                auto is_sheriff_or_deputy = [](player_ptr p) { return p->m_role == player_role::sheriff || p->m_role == player_role::deputy; };

                if (rn::none_of(alive_players, is_sheriff)) {
                    if (rn::distance(alive_players) == 1 && is_renegade(alive_players.front())) {
                        declare_winners(alive_players);
                    } else {
                        declare_winners(rv::filter(target->m_game->m_players, is_outlaw));
                    }
                } else if (rn::all_of(alive_players, is_sheriff_or_deputy)) {
                    declare_winners(rv::filter(target->m_game->m_players, is_sheriff_or_deputy));
                }
            } else {
                if (rn::distance(alive_players) <= 1) {
                    declare_winners(alive_players);
                } else if (killer && !target->alive() && (
                    (target->m_role == player_role::outlaw_3p && killer->m_role == player_role::renegade_3p) ||
                    (target->m_role == player_role::renegade_3p && killer->m_role == player_role::deputy_3p) ||
                    (target->m_role == player_role::deputy_3p && killer->m_role == player_role::outlaw_3p)))
                {
                    declare_winners(rv::single(killer));
                }
            }
        }, -8);
    }
}