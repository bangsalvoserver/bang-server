#include "vera_custer.h"

#include "effects/base/pick.h"

#include "cards/filter_enums.h"
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

    struct request_vera_custer : request_picking_player {
        request_vera_custer(card_ptr origin_card, player_ptr target)
            : request_picking_player(origin_card, nullptr, target, {}, -25) {}

        void on_update() override {
            if (target->alive() && target->m_game->m_playing == target && !target->m_game->is_disabled(origin_card)) {
                auto_pick();
            } else {
                target->m_game->pop_request();
            }
        }

        bool can_pick(const_player_ptr target_player) const override {
            return target_player != target;
        }

        void on_pick(player_ptr target_player) {
            target->m_game->pop_request();

            std::span<card_ptr> target_characters = target_player->m_characters;
            if (target_characters.size() > 1) {
                // Handle Greygory Deck
                target_characters = target_characters.subspan(1);
            }

            auto new_cards = target_characters | rv::transform(get_card_copy) | rn::to_vector;
            target->m_game->add_cards_to(new_cards, pocket_type::player_character, target, card_visibility::shown);
            
            for (card_ptr target_card : new_cards) {
                target->m_game->add_log("LOG_COPY_CHARACTER", target, target_card);
                target->enable_equip(target_card);
            }

            event_card_key key{origin_card, 10};
            target->m_game->add_listener<event_type::pre_turn_start>(key, [=, target=target](player_ptr origin){
                if (origin == target) {
                    target->m_game->remove_listeners(key);
                    target->m_game->queue_action([=]{
                        target->remove_cards({target->m_characters.begin() + 1, target->m_characters.end()});
                    }, -24);
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
        origin->m_game->add_listener<event_type::pre_turn_start>(origin_card, [=](player_ptr target) {
            if (origin == target) {
                origin->m_game->queue_request<request_vera_custer>(origin_card, target);
            }
        });
    }

    void equip_vera_custer::on_disable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->remove_listeners(event_card_key{ origin_card, 0 });
    }

}