#include "requests.h"

#include "cards/game_enums.h"

#include "game/game.h"
#include "game/game_options.h"

namespace banggame {

    void request_characterchoice::on_update() {
        if (!target->m_game->m_options.character_choice) {
            on_pick(target->m_hand.front());
        }
    }

    bool request_characterchoice::can_pick(const_card_ptr target_card) const {
        return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
    }

    void request_characterchoice::on_pick(card_ptr target_card) {
        bool instant = !target->m_game->m_options.character_choice;

        target->m_game->pop_request();
        target->m_game->add_log("LOG_CHARACTER_CHOICE", target, target_card);
        target_card->move_to(pocket_type::player_character, target, card_visibility::shown, instant);
        target->m_hand.front()->move_to(pocket_type::player_backup, target, card_visibility::hidden, true);

        target->reset_max_hp();
        target->set_hp(target->m_max_hp, instant);
        
        target->enable_equip(target_card);
    }

    game_string request_characterchoice::status_text(player_ptr owner) const {
        if (owner == target) {
            return "STATUS_CHARACTERCHOICE";
        } else {
            return {"STATUS_CHARACTERCHOICE_OTHER", target};
        }
    }

    void request_discard::on_update() {
        if (!target->alive()) {
            target->m_game->pop_request();
        } else if (rn::none_of(target->m_hand, [&](const_card_ptr c) { return can_pick(c); })) {
            target->m_game->pop_request();
            target->reveal_hand();
        } else if (target->m_hand.size() == 1) {
            auto_pick();
        }
    }

    bool request_discard::can_pick(const_card_ptr target_card) const {
        return target_card->pocket == pocket_type::player_hand && target_card->owner == target
            && !target->m_game->is_usage_disabled(target_card);
    }
    
    void request_discard::on_pick(card_ptr target_card) {
        target->m_game->pop_request();
        target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
        target->discard_used_card(target_card);
    }

    game_string request_discard::status_text(player_ptr owner) const {
        if (target == owner) {
            return {"STATUS_DISCARD", origin_card};
        } else {
            return {"STATUS_DISCARD_OTHER", target, origin_card};
        }
    }

    struct request_discard_hand_pass : request_discard_pass, interface_resolvable {
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
            target->m_game->queue_request<request_discard_hand_pass>(target);
        }
    }

    bool request_discard_pass::can_pick(const_card_ptr target_card) const {
        return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
    }

    void request_discard_pass::on_pick(card_ptr target_card) {
        target->m_game->add_log("LOG_DISCARDED_SELF_CARD", target, target_card);
        if (target->m_game->check_flags(game_flag::phase_one_draw_discard)) {
            target_card->move_to(pocket_type::main_deck, nullptr, card_visibility::hidden);
        } else {
            target->discard_card(target_card);
        }
        ++ndiscarded;
        target->m_game->call_event(event_type::on_discard_pass{ target, target_card });
        if (target->m_hand.size() <= target->max_cards_end_of_turn()) {
            target->m_game->pop_request();
            target->m_game->call_event(event_type::post_discard_pass{ target, ndiscarded });
            target->m_game->queue_action([target = target]{ target->pass_turn(); }, 1);
        }
    }

    game_string request_discard_pass::status_text(player_ptr owner) const {
        int diff = int(target->m_hand.size()) - target->max_cards_end_of_turn();
        if (target == owner) {
            return {"STATUS_DISCARD_PASS", diff};
        } else {
            return {"STATUS_DISCARD_PASS_OTHER", target, diff};
        }
    }

    static bool is_valid_card(const_card_ptr target_card) {
        return !target_card->is_black() && !target_card->is_train();
    }
        
    bool request_discard_all::can_pick(const_card_ptr target_card) const {
        return (target_card->pocket == pocket_type::player_hand || target_card->pocket == pocket_type::player_table)
            && target_card->owner == target && is_valid_card(target_card);
    }

    void request_discard_all::on_pick(card_ptr target_card) {
        target->m_game->add_log("LOG_DISCARDED_SELF_CARD", target, target_card);
        target->discard_card(target_card);
    }

    void request_discard_all::on_update() {
        if (!live) {
            for (card_ptr target_card : target->m_table) {
                target_card->set_inactive(false);
            }
        }
        
        if (target->m_game->m_options.quick_discard_all
            || (rn::count_if(target->m_table, is_valid_card) + target->m_hand.size()) <= 1)
        {
            on_resolve();
        }
    }

    void request_discard_all::on_resolve() {
        target->m_game->pop_request();

        card_list cards_to_discard = rv::concat(target->m_table, target->m_hand) | rn::to_vector;
        rn::stable_partition(cards_to_discard, is_valid_card);

        for (card_ptr target_card : cards_to_discard) {
            on_pick(target_card);
        }
        
        target->first_character()->drop_cubes();
        if (reason != discard_all_reason::sheriff_killed_deputy) {
            target->add_gold(-target->m_gold);
        }
        if (reason == discard_all_reason::death) {
            target->m_game->play_sound("death");
        }
    }

    game_string request_discard_all::status_text(player_ptr owner) const {
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
                return {"STATUS_SHERIFF_KILLED_DEPUTY_OTH", target};
            }
        }
    }
        
    bool request_discard_hand::can_pick(const_card_ptr target_card) const {
        return target_card->pocket == pocket_type::player_hand && target_card->owner == target;
    }

    void request_discard_hand::on_pick(card_ptr target_card) {
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

    game_string request_discard_hand::status_text(player_ptr owner) const {
        if (target == owner) {
            return {"STATUS_DISCARD_HAND", origin_card};
        } else {
            return {"STATUS_DISCARD_HAND_OTHER", origin_card, target};
        }
    }


}