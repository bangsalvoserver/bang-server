#include "vera_custer.h"

#include "effects/base/pick.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    namespace event_type {
        struct get_card_copy {
            card_ptr target_card;
            nullable_ref<card_ptr> result;
        };
    }

    static card_ptr get_card_copy(card_ptr target_card)  {
        card_ptr result = nullptr;
        target_card->m_game->call_event(event_type::get_card_copy{ target_card, result });
        if (!result) {
            result = target_card->m_game->add_card(*target_card);
            target_card->m_game->add_listener<event_type::get_card_copy>(nullptr, [=](card_ptr e_target_card, card_ptr &e_result) {
                if (e_target_card == target_card) {
                    e_result = result;
                }
            });
        }
        return result;
    }

    struct request_vera_custer : request_picking {
        using request_picking::request_picking;

        void on_update() override {
            auto_pick();
        }

        bool can_pick(const_card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_character
                && target_card->owner != target
                && (target_card->owner->m_characters.size() <= 1
                    || target_card != target_card->owner->get_character());
            // Disallows picking Greygory Deck
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();

            card_ptr card_copy = get_card_copy(target_card);
            target->m_game->add_cards_to({ card_copy }, pocket_type::player_character, target, card_visibility::shown);
            
            target->m_game->add_log("LOG_COPY_CHARACTER", target, card_copy);
            target->enable_equip(card_copy);

            event_card_key key{origin_card, 10};
            target->m_game->add_listener<event_type::pre_turn_start>(key, [=, target=target](player_ptr origin){
                if (origin == target && !target->check_player_flags(player_flag::extra_turn)) {
                    target->m_game->remove_listeners(key);
                    target->m_game->queue_action([=]{
                        target->remove_cards({ card_copy });
                    }, -23);
                }
            });
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_VERA_CUSTER", origin_card};
            } else {
                return {"STATUS_VERA_CUSTER_OTHER", target, origin_card};
            }
        }
    };

    void equip_vera_custer::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::check_character_modifier>({origin_card, 0}, [=](player_ptr target, bool &handled, std::set<card_ptr> &handlers) {
            if (origin == target && !handled && !handlers.contains(origin_card)
                && !target->check_player_flags(player_flag::extra_turn)
            ) {
                handled = true;
                handlers.insert(origin_card);
                target->m_game->queue_request<request_vera_custer>(origin_card, nullptr, target);
            }
        });
    }

    void equip_vera_custer::on_disable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->remove_listeners(event_card_key{ origin_card, 0 });
    }

}
