#include "newidentity.h"

#include "effects/base/pick.h"

#include "cards/game_events.h"

#include "game/game_table.h"

#include "utils/random_element.h"

namespace banggame {

    struct request_newidentity : request_picking {
        request_newidentity(card_ptr origin_card, player_ptr target, card_ptr choice_card)
            : request_picking(origin_card, nullptr, target, {}, -20)
            , choice_card{choice_card} {}
        
        card_ptr choice_card;

        void on_update() override {
            if (target->alive() && target->m_game->m_playing == target) {
                if (update_count == 0) {
                    target->m_game->add_cards_to({ choice_card }, pocket_type::selection, nullptr, card_visibility::shown);
                }
            } else {
                target->m_game->pop_request();
            }
        }

        bool can_pick(const_card_ptr target_card) const override {
            return target_card == choice_card || target_card == target->get_character();
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            if (target_card == choice_card) {
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

    void equip_newidentity::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_turn_switch>({origin_card, -1}, [=, selected_cards = card_set{}](player_ptr target) mutable {
            if (auto possible_cards = origin->m_game->m_characters | rv::filter([&](card_ptr c) {
                return c->pocket == pocket_type::none && c->owner == nullptr && !selected_cards.contains(c);
            })) {
                card_ptr choice_card = random_element(possible_cards, origin->m_game->rng);
                selected_cards.add(choice_card);
                origin->m_game->queue_request<request_newidentity>(origin_card, target, choice_card);
            }
        });
    }
}