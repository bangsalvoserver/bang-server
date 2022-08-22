#include "equips.h"

#include "../../game.h"

namespace banggame {

    void event_based_effect::on_disable(card *target_card, player *target) {
        target->m_game->remove_listeners(target_card);
    }

    void effect_mustang::on_enable(card *target_card, player *target) {
        ++target->m_distance_mod;
        target->send_player_status();
    }

    void effect_mustang::on_disable(card *target_card, player *target) {
        --target->m_distance_mod;
        target->send_player_status();
    }

    void effect_scope::on_enable(card *target_card, player *target) {
        ++target->m_range_mod;
        target->send_player_status();
    }

    void effect_scope::on_disable(card *target_card, player *target) {
        --target->m_range_mod;
        target->send_player_status();
    }

    opt_game_str effect_prompt_on_self_equip::on_prompt(player *origin, card *target_card, player *target) {
        if (target == origin) {
            return game_string{"PROMPT_EQUIP_ON_SELF", target_card};
        } else {
            return std::nullopt;
        }
    }

    void effect_predraw_check::on_enable(card *target_card, player *target) {
        target->m_predraw_checks.try_emplace(target_card, priority, false);
    }

    void effect_predraw_check::on_disable(card *target_card, player *target) {
        target->m_predraw_checks.erase(target_card);
    }

    void effect_jail::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player *p, card *e_card) {
            if (p == target && e_card == target_card) {
                target->m_game->draw_check_then(target, target_card, [=](card *drawn_card) {
                    target->discard_card(target_card);
                    if (target->get_card_sign(drawn_card).suit == card_suit::hearts) {
                        target->m_game->add_log("LOG_JAIL_BREAK", target);
                    } else {
                        target->m_game->add_log("LOG_SKIP_TURN", target);
                        target->skip_turn();
                    }
                });
            }
        });
    }

    void effect_dynamite::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player *e_player, card *e_card) {
            if (e_player == target && e_card == target_card) {
                target->m_game->draw_check_then(target, target_card, [=](card *drawn_card) {
                    card_sign sign = target->get_card_sign(drawn_card);
                    if (sign.suit == card_suit::spades
                        && enums::indexof(sign.rank) >= enums::indexof(card_rank::rank_2)
                        && enums::indexof(sign.rank) <= enums::indexof(card_rank::rank_9)) {
                        target->m_game->add_log("LOG_CARD_EXPLODES", target_card);
                        target->discard_card(target_card);
                        target->damage(target_card, nullptr, 3);
                    } else if (auto dest = std::ranges::find_if(range_other_players(target), [target_card](player &p) {
                        return !p.find_equipped_card(target_card);
                    }); dest != target) {
                        target_card->on_disable(target);
                        dest->equip_card(target_card);
                    }
                });
            }
        });
    }

    static bool is_horse(const card *c) {
        return c->has_tag(tag_type::horse);
    }

    opt_game_str effect_horse::on_prompt(player *origin, card *target_card, player *target) {
        if (auto it = std::ranges::find_if(target->m_table, is_horse); it != target->m_table.end()) {
            return game_string{"PROMPT_REPLACE", target_card, *it};
        } else {
            return std::nullopt;
        }
    }

    void effect_horse::on_equip(card *target_card, player *target) {
        if (auto it = std::ranges::find_if(target->m_table, is_horse); it != target->m_table.end()) {
            target->discard_card(*it);
        }
    }
    
    static bool is_weapon(card *c) {
        return c->has_tag(tag_type::weapon);
    }

    opt_game_str effect_weapon::on_prompt(player *origin, card *target_card, player *target) {
        if (target == origin) {
            if (auto it = std::ranges::find_if(target->m_table, is_weapon); it != target->m_table.end()) {
                return game_string{"PROMPT_REPLACE", target_card, *it};
            }
        }
        return std::nullopt;
    }

    void effect_weapon_base::on_equip(card *target_card, player *target) {
        if (auto it = std::ranges::find_if(target->m_table, is_weapon); it != target->m_table.end()) {
            target->m_game->add_log("LOG_DISCARDED_SELF_CARD", target, *it);
            target->discard_card(*it);
        }
    }

    void effect_weapon_base::on_enable(card *target_card, player *target) {
        target->m_weapon_range = range;
        target->send_player_status();
    }

    void effect_weapon_base::on_disable(card *target_card, player *target) {
        target->m_weapon_range = 1;
        target->send_player_status();
    }

    void effect_volcanic::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::apply_volcanic_modifier>(target_card, [=](player *p, bool &value) {
            value = value || p == target;
        });
    }

    void effect_boots::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_hit>({target_card, 1}, [p, target_card](card *origin_card, player *origin, player *target, int damage, bool is_bang) {
            if (p == target) {
                target->m_game->queue_action([=]{
                    if (target->alive()) {
                        target->m_game->flash_card(target_card);
                        target->draw_card(damage, target_card);
                    }
                });
            }
        });
    }

    void effect_horseshoe::on_enable(card *target_card, player *target) {
        ++target->m_num_checks;
    }

    void effect_horseshoe::on_disable(card *target_card, player *target) {
        --target->m_num_checks;
    }

    void effect_initialcards::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::apply_initial_cards_modifier>(target_card, [target, value=value](player *p, int &value_ref) {
            if (p == target) {
                value_ref = value;
            }
        });
    }
}