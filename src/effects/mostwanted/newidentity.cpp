#include "newidentity.h"

#include "effects/base/pick.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    static card_ptr get_random_character(player_ptr origin) {
        return random_element(origin->m_game->get_all_cards()
            | rv::filter([&](card_ptr target_card) {
                return target_card->deck == card_deck_type::character
                    && target_card->owner == nullptr;
            }), origin->m_game->rng);
    }

    struct request_newidentity : request_picking {
        request_newidentity(card_ptr origin_card, player_ptr target)
            : request_picking(origin_card, nullptr, target, {}, -20) {}

        void on_update() override {
            if (target->alive() && target->m_game->m_playing == target) {
                if (!live) {
                    target->m_game->add_cards_to({ get_random_character(target) }, pocket_type::selection, nullptr, card_visibility::shown);
                }
            } else {
                target->m_game->pop_request();
            }
        }

        bool can_pick(const_card_ptr target_card) const override {
            return target_card->pocket == pocket_type::selection
                || (target_card->pocket == pocket_type::player_character && target_card == target->get_character());
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            if (target_card->pocket == pocket_type::selection) {
                target->m_game->add_log("LOG_CHARACTER_CHOICE", target, target_card);
                target->set_character(target_card);
                
                if (!target->is_ghost()) {
                    target->set_hp(2);
                }
            } else {
                target->m_game->remove_cards(target->m_game->m_selection);
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_NEWIDENTITY", origin_card};
            } else {
                return {"STATUS_NEWIDENTITY_OTHER", target, origin_card};
            }
        }
    };

    void equip_newidentity::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_switch>({target_card, -1}, [=](player_ptr origin) {
            target->m_game->queue_request<request_newidentity>(target_card, origin);
        });
    }
}