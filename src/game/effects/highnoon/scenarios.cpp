#include "scenarios.h"

#include "../../game.h"

namespace banggame {
    using namespace enums::flag_operators;

    void effect_blessing::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::apply_sign_modifier>(target_card, [](player *, card_sign &value) {
            value.suit = card_suit::hearts;
        });
    }

    void effect_curse::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::apply_sign_modifier>(target_card, [](player *, card_sign &value) {
            value.suit = card_suit::spades;
        });
    }

    void effect_thedaltons::on_enable(card *target_card, player *target) {
        for (player &p : range_all_players(target)) {
            if (std::ranges::find(p.m_table, card_color_type::blue, &card::color) != p.m_table.end()) {
                p.m_game->queue_request<request_thedaltons>(target_card, &p);
            }
        }
    }

    bool request_thedaltons::can_pick(pocket_type pocket, player *target_player, card *target_card) const {
        return pocket == pocket_type::player_table
            && target == target_player
            && target_card->color == card_color_type::blue;
    }

    void request_thedaltons::on_pick(pocket_type pocket, player *target_player, card *target_card) {
        target->m_game->pop_request();
        target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
        target->discard_card(target_card);
        target->m_game->update_request();
    }

    game_string request_thedaltons::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_THEDALTONS", origin_card};
        } else {
            return {"STATUS_THEDALTONS_OTHER", target, origin_card};
        }
    }

    void effect_thedoctor::on_enable(card *target_card, player *target) {
        int min_hp = std::ranges::min(target->m_game->m_players
            | std::views::filter(&player::alive)
            | std::views::transform(&player::m_hp));
        
        for (player &p : range_all_players(target)) {
            if (p.m_hp == min_hp) {
                p.heal(1);
            }
        }
    }

    void effect_trainarrival::on_enable(card *target_card, player *target) {
        for (auto &p : target->m_game->m_players) {
            ++p.m_num_cards_to_draw;
        }
    }

    void effect_trainarrival::on_disable(card *target_card, player *target) {
        for (auto &p : target->m_game->m_players) {
            --p.m_num_cards_to_draw;
        }
    }

    void effect_thirst::on_enable(card *target_card, player *target) {
        for (auto &p : target->m_game->m_players) {
            --p.m_num_cards_to_draw;
        }
    }

    void effect_thirst::on_disable(card *target_card, player *target) {
        for (auto &p : target->m_game->m_players) {
            ++p.m_num_cards_to_draw;
        }
    }

    void effect_highnoon::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::pre_turn_start>(target_card, [=](player *p) {
            p->damage(target_card, nullptr, 1);
        });
    }

    void effect_shootout::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_start>(target_card, [](player *p) {
            ++p->m_bangs_per_turn;
        });
    }

    void effect_invert_rotation::on_enable(card *target_card, player *target) {
        target->m_game->set_game_flags(game_flags::invert_rotation);
    }

    void effect_reverend::on_enable(card *target_card, player *target) {
        target->m_game->add_disabler(target_card, [](card *c) {
            return c->pocket == pocket_type::player_hand
                && c->has_tag(tag_type::beer);
        });
    }

    void effect_reverend::on_disable(card *target_card, player *target) {
        target->m_game->remove_disablers(target_card);
    }

    void effect_hangover::on_enable(card *target_card, player *target) {
        target->m_game->add_disabler(target_card, [](card *c) {
            return c->pocket == pocket_type::player_character;
        });
    }

    void effect_hangover::on_disable(card *target_card, player *target) {
        target->m_game->remove_disablers(target_card);
        target->m_game->call_event<event_type::on_effect_end>(target, target_card);
    }

    void effect_sermon::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::pre_turn_start>(target_card, [=](player *p) {
            target->m_game->add_disabler(target_card, [=](card *c) {
                if (c->owner != p) {
                    return false;
                } else if (c->pocket == pocket_type::player_hand) {
                    return p->is_bangcard(c);
                } else {
                    return c->has_tag(tag_type::play_as_bang);
                }
            });
        });
        target->m_game->add_listener<event_type::on_turn_end>(target_card, [=](player *p) {
            target->m_game->remove_disablers(target_card);
        });
    }

    void effect_sermon::on_disable(card *target_card, player *target) {
        target->m_game->remove_disablers(target_card);
        target->m_game->remove_listeners(target_card);
    }

    void effect_ghosttown::on_enable(card *target_card, player *origin) {
        origin->m_game->add_listener<event_type::verify_revivers>(target_card,
            [=, last_revived = static_cast<player*>(nullptr)](player *target) mutable {
                if (last_revived) {
                    last_revived->remove_player_flags(player_flags::temp_ghost);
                    --last_revived->m_num_cards_to_draw;
                    if (!last_revived->alive()) {
                        origin->m_game->handle_player_death(nullptr, last_revived, true);
                    }
                    last_revived = nullptr;
                }
                if (!target->alive()) {
                    origin->m_game->add_log("LOG_REVIVE", target, target_card);

                    target->add_player_flags(player_flags::temp_ghost);
                    ++target->m_num_cards_to_draw;
                    
                    for (auto *c : target->m_characters) {
                        c->on_enable(target);
                    }
                    
                    last_revived = target;
                }
            });
    }

    void effect_handcuffs::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::post_draw_cards>(target_card, [=](player *origin) {
            std::vector<card *> target_cards;
            for (card *c : origin->m_game->m_hidden_deck) {
                if (c->has_tag(tag_type::handcuffs)) {
                    target_cards.push_back(c);
                }
            }
            for (card *c : target_cards) {
                origin->m_game->move_card(c, pocket_type::selection, nullptr, show_card_flags::instant);
            }
            origin->m_game->queue_request<request_handcuffs>(target_card, origin);
        });
        target->m_game->add_listener<event_type::on_turn_end>(target_card, [target_card](player *p) {
            p->m_game->remove_listeners(event_card_key{target_card, 1});
        });
    }

    void request_handcuffs::on_pick(pocket_type pocket, player *target_player, card *target_card) {
        target->m_game->flash_card(target_card);
        
        auto declared_suit = static_cast<card_suit>(*target_card->get_tag_value(tag_type::handcuffs));
        switch (declared_suit) {
        case card_suit::clubs:
            target->m_game->add_log("LOG_DECLARED_CLUBS", target, origin_card);
            break;
        case card_suit::diamonds:
            target->m_game->add_log("LOG_DECLARED_DIAMONDS", target, origin_card);
            break;
        case card_suit::hearts:
            target->m_game->add_log("LOG_DECLARED_HEARTS", target, origin_card);
            break;
        case card_suit::spades:
            target->m_game->add_log("LOG_DECLARED_SPADES", target, origin_card);
            break;
        }

        target->m_game->add_listener<event_type::verify_play_card>({origin_card, 1},
            [origin_card=origin_card, target=target, declared_suit](player *origin, card *c, game_string &out_error) {
                if (c->owner == target && c->sign && c->sign.suit != declared_suit) {
                    out_error = {"ERROR_INVALID_SUIT", origin_card, c};
                }
            });

        target->m_game->pop_request();
        while (!target->m_game->m_selection.empty()) {
            target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::hidden_deck, nullptr, show_card_flags::instant);
        }
        target->m_game->update_request();
    }

    game_string request_handcuffs::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_HANDCUFFS", origin_card};
        } else {
            return {"STATUS_HANDCUFFS_OTHER", target, origin_card};
        }
    }

    void effect_newidentity::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::pre_turn_start>(target_card, [=](player *p) {
            target->m_game->move_card(p->m_backup_character.front(), pocket_type::selection);
            target->m_game->queue_request<request_newidentity>(target_card, p);
        });
    }

    bool request_newidentity::can_pick(pocket_type pocket, player *, card *target_card) const {
        return pocket == pocket_type::selection
            || (pocket == pocket_type::player_character && target_card == target->m_characters.front());
    }

    void request_newidentity::on_pick(pocket_type pocket, player *, card *target_card) {
        target->m_game->pop_request();
        if (pocket == pocket_type::selection) {
            for (card *c : target->m_characters) {
                target->disable_equip(c);
            }
            if (target->m_characters.size() > 1) {
                target->m_game->add_update<game_update_type::remove_cards>(to_vector(target->m_characters | std::views::drop(1)));
                target->m_characters.resize(1);
            }

            target->m_game->add_log("LOG_CHARACTER_CHOICE", target, target_card);

            card *old_character = target->m_characters.front();
            target->m_game->move_card(old_character, pocket_type::player_backup, target, show_card_flags::hidden);
            target->m_game->move_card(target_card, pocket_type::player_character, target, show_card_flags::shown);

            target->reset_max_hp();
            target->enable_equip(target_card);
            target->move_cubes(old_character, target_card, old_character->num_cubes);
            target_card->on_equip(target);
            
            target->set_hp(2);
        } else {
            target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::player_backup, target, show_card_flags::hidden);
        }
        target->m_game->update_request();
    }

    game_string request_newidentity::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_NEWIDENTITY", origin_card};
        } else {
            return {"STATUS_NEWIDENTITY_OTHER", target, origin_card};
        }
    }
}