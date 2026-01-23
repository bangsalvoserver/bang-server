#include "vera_custer.h"

#include "effects/base/pick.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    namespace event_type {
        struct get_card_copy {
            using result_type = card_ptr;
            card_ptr target_card;
        };
    }

    static card_ptr get_card_copy(card_ptr target_card)  {
        card_ptr result = target_card->m_game->call_event(event_type::get_card_copy{ target_card });
        if (!result) {
            result = target_card->m_game->add_card(*target_card);
            target_card->m_game->add_listener<event_type::get_card_copy>(nullptr, [=](card_ptr e_target_card) -> card_ptr {
                if (e_target_card == target_card) {
                    return result;
                }
                return nullptr;
            });
        }
        return result;
    }

    static void resolve_vera_custer(card_ptr origin_card, player_ptr origin, player_ptr target) {
        size_t start_index = origin->m_characters.size();

        card_ptr target_card = get_card_copy(target->get_character());
        origin->m_game->add_cards_to({ target_card }, pocket_type::player_character, origin, card_visibility::shown);
        
        origin->m_game->add_log("LOG_COPY_CHARACTER", origin, target_card);
        origin->enable_equip(target_card);

        event_card_key key{origin_card, 10};
        origin->m_game->add_listener<event_type::pre_turn_start>(key, [=](player_ptr e_origin){
            if (origin == e_origin && !origin->check_player_flags(player_flag::extra_turn)) {
                origin->m_game->remove_listeners(key);
                origin->m_game->queue_action([=]{
                    if (rn::contains(origin->m_characters, origin_card)) {
                        origin->remove_characters(start_index);
                    }
                }, -23);
            }
        });
    }

    struct request_vera_custer : request_picking_player {
        using request_picking_player::request_picking_player;

        void on_update() override {
            if (auto targetable = target->m_game->m_players | rv::filter([&](player_ptr target_player) {
                return target_player != target && target_player->alive();
            })) {
                auto first_target = targetable.front();
                if (rn::all_of(targetable, [&](player_ptr target_player) {
                    return target_player->get_character()->name == first_target->get_character()->name;
                })) {
                    on_pick(first_target);
                }
            }
        }

        bool can_pick(const_player_ptr target_player) const override {
            return target_player != target;
        }

        void on_pick(player_ptr target_player) override {
            target->m_game->pop_request();
            resolve_vera_custer(origin_card, target, target_player);
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
        origin->m_game->add_listener<event_type::check_character_modifier>({origin_card, 0}, [=](player_ptr target, card_set &handlers) {
            if (origin == target && !handlers.contains(origin_card)
                && !target->check_player_flags(player_flag::extra_turn)
            ) {
                handlers.add(origin_card);
                target->m_game->queue_request<request_vera_custer>(origin_card, nullptr, target);
                return true;
            }
            return false;
        });
    }

    void equip_vera_custer::on_disable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->remove_listeners(event_card_key{ origin_card, 0 });
    }

}
