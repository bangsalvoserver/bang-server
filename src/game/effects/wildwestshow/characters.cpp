#include "characters.h"

#include "../base/requests.h"

#include "../../game.h"

namespace banggame {
    using namespace enums::flag_operators;

    void effect_big_spencer::on_enable(card *target_card, player *p) {
        p->m_game->add_disabler(target_card, [=](card *c) {
            return c->pocket == pocket_type::player_hand
                && c->owner == p
                && c->has_tag(tag_type::missedcard);
        });
    }

    void effect_big_spencer::on_disable(card *target_card, player *p) {
        p->m_game->remove_disablers(target_card);
    }

    void effect_gary_looter::on_enable(card *target_card, player *player_end) {
        player_end->m_game->add_listener<event_type::verify_card_taker>(target_card, [=](player *e_target, equip_type type, bool &value) {
            if (type == equip_type::gary_looter && e_target == player_end) {
                value = true;
            }
        });
        player_end->m_game->add_listener<event_type::on_discard_pass>(target_card, [=](player *player_begin, card *discarded_card) {
            const auto is_valid_target = [=](player &target) {
                return target.m_game->call_event<event_type::verify_card_taker>(&target, equip_type::gary_looter, false);
            };
            if (player_begin != player_end && std::none_of(player_iterator(player_begin), player_iterator(player_end), is_valid_target)) {
                player_end->m_game->add_log("LOG_DRAWN_CARD", player_end, discarded_card);
                player_end->m_game->move_card(discarded_card, pocket_type::player_hand, player_end, show_card_flags::short_pause);
            }
        });
    }

    void effect_john_pain::on_enable(card *target_card, player *player_end) {
        player_end->m_game->add_listener<event_type::verify_card_taker>(target_card, [=](player *e_target, equip_type type, bool &value) {
            if (type == equip_type::john_pain && e_target == player_end && e_target->m_hand.size() < 6) {
                value = true;
            }
        });
        player_end->m_game->add_listener<event_type::on_draw_check>(target_card, [=](player *player_begin, card *drawn_card) {
            const auto is_valid_target = [=](player &target) {
                return target.m_game->call_event<event_type::verify_card_taker>(&target, equip_type::john_pain, false);
            };
            if (drawn_card->pocket != pocket_type::player_hand
                && std::none_of(player_iterator(player_begin), player_iterator(player_end), is_valid_target)
                && is_valid_target(*player_end))
            {
                player_end->m_game->add_log("LOG_DRAWN_CARD", player_end, drawn_card);
                player_end->m_game->move_card(drawn_card, pocket_type::player_hand, player_end, show_card_flags::short_pause);
            }
        });
    }

    void effect_teren_kill::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_player_death_resolve>(origin_card, [=](player *target, bool tried_save) {
            if (origin == target && !tried_save) {
                origin->m_game->draw_check_then(origin, origin_card, [=](card *drawn_card) {
                    if (origin->get_card_sign(drawn_card).suit != card_suit::spades) {
                        origin->set_hp(1);
                        origin->draw_card(1, origin_card);
                    }
                });
            }
        });
    }

    void effect_youl_grinner::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player *origin) {
            if (target == origin) {
                for (player &p : range_other_players(target)) {
                    if (p.m_hand.size() > target->m_hand.size()) {
                        target->m_game->queue_request<request_youl_grinner>(target_card, target, &p);
                    }
                }
            }
        });
    }

    bool request_youl_grinner::can_pick(pocket_type pocket, player *target_player, card *target_card) const {
        return pocket == pocket_type::player_hand && target_player == target;
    }

    void request_youl_grinner::on_pick(pocket_type pocket, player *target_player, card *target_card) {
        target->m_game->pop_request();
        target->m_game->add_log(update_target::includes(origin, target), "LOG_GIFTED_CARD", target, origin, target_card);
        target->m_game->add_log(update_target::excludes(origin, target), "LOG_GIFTED_A_CARD", target, origin);
        origin->steal_card(target_card);
        target->m_game->call_event<event_type::on_effect_end>(origin, origin_card);
        target->m_game->update_request();
    }

    game_string request_youl_grinner::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_YOUL_GRINNER", origin_card};
        } else {
            return {"STATUS_YOUL_GRINNER_OTHER", target, origin_card};
        }
    }

    void handler_flint_westwood::on_play(card *origin_card, player *origin, card *chosen_card, card *target_card) {
        auto target = target_card->owner;

        for (int i=2; i && !target->m_hand.empty(); --i) {
            card *stolen_card = target->random_hand_card();
            target->m_game->add_log(update_target::includes(origin, target), "LOG_STOLEN_CARD", origin, target, stolen_card);
            target->m_game->add_log(update_target::excludes(origin, target), "LOG_STOLEN_CARD_FROM_HAND", origin, target);
            origin->steal_card(stolen_card);
        }
        target->m_game->add_log(update_target::includes(origin, target), "LOG_GIFTED_CARD", origin, target, chosen_card);
        target->m_game->add_log(update_target::excludes(origin, target), "LOG_GIFTED_A_CARD", origin, target);
        target->steal_card(chosen_card);
    }

    void effect_greygory_deck::on_equip(card *target_card, player *target) {
        std::vector<card *> base_characters;
        for (card &c : target->m_game->m_cards) {
            if (c.expansion == card_expansion_type{}
                && (c.pocket == pocket_type::none
                || (c.pocket == pocket_type::player_character && c.owner == target)))
                base_characters.push_back(&c);
        }
        std::ranges::shuffle(base_characters, target->m_game->rng);

        target->m_game->add_update<game_update_type::add_cards>(
            make_id_vector(base_characters | std::views::take(2)),
            pocket_type::player_character, target);
        for (int i=0; i<2; ++i) {
            auto *c = target->m_characters.emplace_back(base_characters[i]);
            target->m_game->add_log("LOG_CHARACTER_CHOICE", target, c);
            c->pocket = pocket_type::player_character;
            c->owner = target;
            c->on_enable(target);
            target->m_game->send_card_update(c, target, show_card_flags::instant | show_card_flags::shown);
        }
    }
    
    void effect_greygory_deck::on_play(card *target_card, player *target) {
        for (int i=1; i<target->m_characters.size(); ++i) {
            auto *c = target->m_characters[i];
            target->disable_equip(c);
            c->pocket = pocket_type::none;
            c->owner = nullptr;
        }
        if (target->m_characters.size() > 1) {
            target->m_game->add_update<game_update_type::remove_cards>(std::span(target->m_characters).subspan(1));
            target->m_characters.resize(1);
        }
        on_equip(target_card, target);
        target->m_game->update_request();
    }

}