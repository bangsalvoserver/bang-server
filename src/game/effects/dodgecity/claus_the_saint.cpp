#include "claus_the_saint.h"

#include "../../game.h"

namespace banggame {

    struct request_claus_the_saint : selection_picker {
        request_claus_the_saint(card *origin_card, player *target)
            : selection_picker(origin_card, nullptr, target, effect_flags::auto_pick) {}

        player *get_next_target() const {
            return std::next(player_iterator(target),
                target->m_game->num_alive() - static_cast<int>(target->m_game->m_selection.size()));
        }

        void on_pick(pocket_type pocket, player *target_player, card *target_card) override {
            if (target->m_num_drawn_cards < target->get_cards_to_draw()) {
                target->add_to_hand_phase_one(target_card);
            } else {
                player *next_target = get_next_target();
                target->m_game->add_log(update_target::includes(target, next_target), "LOG_GIFTED_CARD", target, next_target, target_card);
                target->m_game->add_log(update_target::excludes(target, next_target), "LOG_GIFTED_A_CARD", target, next_target);
                next_target->add_to_hand(target_card);
            }
            if (target->m_game->m_selection.empty()) {
                target->m_game->pop_request();
            }
            target->m_game->update_request();
        }

        game_string status_text(player *owner) const override {
            if (owner == target) {
                if (target->m_num_drawn_cards < target->get_cards_to_draw()) {
                    return {"STATUS_CLAUS_THE_SAINT_DRAW", origin_card};
                } else {
                    return {"STATUS_CLAUS_THE_SAINT_GIVE", origin_card, get_next_target()};
                }
            } else if (target->m_num_drawn_cards < target->get_cards_to_draw()) {
                return {"STATUS_CLAUS_THE_SAINT_DRAW_OTHER", target, origin_card};
            } else if (auto p = get_next_target(); p != owner) {
                return {"STATUS_CLAUS_THE_SAINT_GIVE_OTHER", target, origin_card, p};
            } else {
                return {"STATUS_CLAUS_THE_SAINT_GIVE_YOU", target, origin_card};
            }
        }
    };
    
    void effect_claus_the_saint::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_draw_from_deck>(target_card, [=](player *origin) {
            if (origin == target) {
                target->m_game->pop_request();
                int ncards = target->m_game->num_alive() + target->get_cards_to_draw() - 1;
                for (int i=0; i<ncards; ++i) {
                    target->m_game->draw_phase_one_card_to(pocket_type::selection, target);
                }
                target->m_game->queue_request<request_claus_the_saint>(target_card, target);
            }
        });
    }
}