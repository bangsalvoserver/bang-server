#include "peyote.h"

#include "effects/base/pick.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    struct request_peyote : selection_picker {
        request_peyote(card *origin_card, player *target)
            : selection_picker(origin_card, nullptr, target) {}

        void on_update() override {
            if (!live) {
                std::vector<card *> target_cards;
                for (card *c : target->m_game->m_hidden_deck) {
                    if (c->has_tag(tag_type::peyote)) {
                        target_cards.push_back(c);
                    }
                }
                for (card *c : target_cards) {
                    c->move_to(pocket_type::selection, nullptr, card_visibility::shown, true);
                }
            }
        }

        void on_pick(card *target_card) override {
            target_card->flash_card();
            
            auto *drawn_card = target->m_game->top_of_deck();
            drawn_card->set_visibility(card_visibility::shown);
            drawn_card->add_short_pause();

            short choice = *target_card->get_tag_value(tag_type::peyote);

            if (choice == 1) {
                target->m_game->add_log("LOG_DECLARED_RED", target, origin_card);
            } else {
                target->m_game->add_log("LOG_DECLARED_BLACK", target, origin_card);
            }

            if ((choice == 1) == drawn_card->sign.is_red()) {
                target->m_game->add_log("LOG_DRAWN_CARD", target, drawn_card);
                target->add_to_hand(drawn_card);
            } else {
                target->m_game->pop_request();
                target->m_game->add_log("LOG_DISCARDED_SELF_CARD", target, drawn_card);
                drawn_card->move_to(pocket_type::discard_pile);

                while (!target->m_game->m_selection.empty()) {
                    target->m_game->m_selection.front()->move_to(pocket_type::hidden_deck, nullptr, card_visibility::shown, true);
                }
            }
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_PEYOTE", origin_card};
            } else {
                return {"STATUS_PEYOTE_OTHER", target, origin_card};
            }
        }
    };
    
    void equip_peyote::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_start>({target_card, -10}, [=](player *p) {
            p->m_game->queue_request<request_peyote>(target_card, p);
        });

        target->m_game->add_game_flags(game_flag::phase_one_override);
    }

    void equip_peyote::on_disable(card *target_card, player *target) {
        target->m_game->remove_listeners(target_card);
        target->m_game->remove_game_flags(game_flag::phase_one_override);
    }
}