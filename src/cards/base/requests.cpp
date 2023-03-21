#include "requests.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    bool request_characterchoice::can_pick(card *target_card) const {
        return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
    }

    void request_characterchoice::on_pick(card *target_card) {
        target->m_game->pop_request();
        target->m_game->add_log("LOG_CHARACTER_CHOICE", target, target_card);
        target->m_game->move_card(target_card, pocket_type::player_character, target, card_visibility::shown);
        target->reset_max_hp();
        target->set_hp(target->m_max_hp, true);
        target->enable_equip(target_card);

        target->m_game->move_card(target->m_hand.front(), pocket_type::player_backup, target, card_visibility::hidden);
    }

    game_string request_characterchoice::status_text(player *owner) const {
        if (owner == target) {
            return "STATUS_CHARACTERCHOICE";
        } else {
            return {"STATUS_CHARACTERCHOICE_OTHER", target};
        }
    }

    bool request_discard::can_pick(card *target_card) const {
        return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
    }
    
    void request_discard::on_pick(card *target_card) {
        if (--ncards == 0) {
            target->m_game->pop_request();
        }
        target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
        target->discard_used_card(target_card);
    }

    game_string request_discard::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_DISCARD", origin_card};
        } else {
            return {"STATUS_DISCARD_OTHER", target, origin_card};
        }
    }

    struct request_discard_hand_pass : request_discard_pass, resolvable_request {
        using request_discard_pass::request_discard_pass;

        void on_update() override {
            if (target->m_game->m_options.quick_discard_all || target->m_hand.size() <= 1) {
                on_resolve();
            }
        }

        void on_resolve() override {
            while (!target->empty_hand()) {
                on_pick(target->m_hand.front());
            }
        }
    };

    void request_discard_pass::on_update() {
        if (target->max_cards_end_of_turn() == 0) {
            target->m_game->pop_request();
            target->m_game->queue_request_front<request_discard_hand_pass>(target);
        }
    }

    bool request_discard_pass::can_pick(card *target_card) const {
        return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
    }

    void request_discard_pass::on_pick(card *target_card) {
        target->m_game->add_log("LOG_DISCARDED_SELF_CARD", target, target_card);
        if (target->m_game->check_flags(game_flags::phase_one_draw_discard)) {
            target->m_game->move_card(target_card, pocket_type::main_deck, nullptr, card_visibility::hidden);
        } else {
            target->discard_card(target_card);
        }
        ++ndiscarded;
        target->m_game->call_event<event_type::on_discard_pass>(target, target_card);
        if (target->m_hand.size() <= target->max_cards_end_of_turn()) {
            target->m_game->pop_request();
            target->m_game->call_event<event_type::post_discard_pass>(target, ndiscarded);
            target->m_game->queue_action([target = target]{ target->pass_turn(); }, 1);
        }
    }

    game_string request_discard_pass::status_text(player *owner) const {
        int diff = int(target->m_hand.size()) - target->max_cards_end_of_turn();
        if (diff > 1) {
            if (target == owner) {
                return {"STATUS_DISCARD_PASS_PLURAL", diff};
            } else {
                return {"STATUS_DISCARD_PASS_PLURAL_OTHER", target, diff};
            }
        } else if (target == owner) {
            return "STATUS_DISCARD_PASS";
        } else {
            return {"STATUS_DISCARD_PASS_OTHER", target};
        }
    }
        
    bool request_discard_all::can_pick(card *target_card) const {
        return (target_card->pocket == pocket_type::player_hand || target_card->pocket == pocket_type::player_table)
            && target_card->owner == target
            && !target_card->is_black();
    }

    void request_discard_all::on_pick(card *target_card) {
        target->m_game->add_log("LOG_DISCARDED_SELF_CARD", target, target_card);
        target->discard_card(target_card);
    }

    void request_discard_all::on_update() {
        if (state == request_state::pending) {
            target->untap_inactive_cards();
        }
        
        if (target->m_game->m_options.quick_discard_all
            || (std::ranges::count_if(target->m_table, std::not_fn(&card::is_black)) + target->m_hand.size()) <= 1)
        {
            on_resolve();
        }
    }

    void request_discard_all::on_resolve() {
        for (card *target_card : ranges::to<std::vector>(ranges::views::concat(
            target->m_table | ranges::views::remove_if(&card::is_black),
            target->m_hand
        ))) {
            on_pick(target_card);
        }

        target->m_game->pop_request();
        
        while (!target->m_table.empty()) {
            on_pick(target->m_table.front());
        }
        target->drop_all_cubes(target->first_character());
        if (reason != discard_all_reason::sheriff_killed_deputy) {
            target->add_gold(-target->m_gold);
        }
        if (reason == discard_all_reason::death) {
            target->m_game->play_sound(nullptr, "death");
        }
    }

    game_string request_discard_all::status_text(player *owner) const {
        if (reason != discard_all_reason::sheriff_killed_deputy) {
            if (target == owner) {
                return "STATUS_DISCARD_ALL";
            } else {
                return {"STATUS_DISCARD_ALL_OTHER", target};
            }
        } else {
            if (target == owner) {
                return "STATUS_SHERIFF_KILLED_DEPUTY";
            } else {
                return {"STATUS_SHERIFF_KILLED_DEPUTY_OTHER", target};
            }
        }
    }
        
    bool request_discard_hand::can_pick(card *target_card) const {
        return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
    }

    void request_discard_hand::on_pick(card *target_card) {
        target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
        target->discard_card(target_card);
    }
    
    void request_discard_hand::on_update() {
        if (target->m_game->m_options.quick_discard_all || target->m_hand.size() <= 1) {
            on_resolve();
        }
    }

    void request_discard_hand::on_resolve() {
        target->m_game->pop_request();
        while (!target->empty_hand()) {
            on_pick(target->m_hand.front());
        }
    }

    game_string request_discard_hand::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_DISCARD_HAND", origin_card};
        } else {
            return {"STATUS_DISCARD_HAND_OTHER", origin_card, target};
        }
    }


}