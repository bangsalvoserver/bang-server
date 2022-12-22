#include "handcuffs.h"

#include "game/game.h"

namespace banggame {
    
    struct request_handcuffs : selection_picker {
        request_handcuffs(card *origin_card, player *target)
            : selection_picker(origin_card, nullptr, target) {}

        void on_pick(card *target_card) override {
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

            target->m_game->invoke_action([&]{
                target->m_game->pop_request();
                while (!target->m_game->m_selection.empty()) {
                    target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::hidden_deck, nullptr, card_visibility::shown, true);
                }
            });
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_HANDCUFFS", origin_card};
            } else {
                return {"STATUS_HANDCUFFS_OTHER", target, origin_card};
            }
        }
    };

    void equip_handcuffs::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_draw_from_deck>({target_card, -1}, [=](player *origin, bool &override_request) {
            std::vector<card *> target_cards;
            for (card *c : origin->m_game->m_hidden_deck) {
                if (c->has_tag(tag_type::handcuffs)) {
                    target_cards.push_back(c);
                }
            }
            for (card *c : target_cards) {
                origin->m_game->move_card(c, pocket_type::selection, nullptr, card_visibility::shown, true);
            }
            origin->m_game->queue_request<request_handcuffs>(target_card, origin);
        });
        target->m_game->add_listener<event_type::on_turn_end>(target_card, [target_card](player *p, bool skipped) {
            p->m_game->remove_listeners(event_card_key{target_card, 1});
        });
    }
}