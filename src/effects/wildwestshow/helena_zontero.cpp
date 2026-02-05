#include "helena_zontero.h"

#include "cards/game_enums.h"

#include "effects/base/draw_check.h"

#include "game/game_table.h"
#include "game/game_options.h"
#include "game/request_timer.h"

namespace banggame {

    struct request_shuffle_roles : request_base, request_timer {
        request_shuffle_roles(card_ptr origin_card, player_ptr origin)
            : request_base(origin_card, origin, nullptr) {}
        
        void on_update() override {
            set_duration(origin->m_game->m_options.auto_resolve_timer);
        }

        void on_finished() override {
            origin->m_game->pop_request();
            
            auto alive_players = rv::filter(origin->m_game->m_players, [](player_ptr p) {
                return p->alive() && !p->is_sheriff();
            });
            
            auto roles = alive_players | rv::transform(&player::get_base_role) | rn::to<std::vector>();
            rn::shuffle(roles, origin->m_game->rng);

            for (auto [p, role] : rv::zip(alive_players, roles)) {
                p->hide_role();
                p->set_role(role);
            }
        }

        game_string status_text(player_ptr owner) const override {
            return "STATUS_ROLES_SHUFFLED";
        }
    };

    struct request_helena_zontero_check : request_base, draw_check_handler, std::enable_shared_from_this<request_helena_zontero_check> {
        request_helena_zontero_check(card_ptr origin_card, player_ptr origin)
            : request_base(origin_card, origin, nullptr) {}

        card_ptr drawn_card = nullptr;

        void on_update() override {
            if (update_count == 0) {
                origin_card->flash_card();
                restart();
            }
        }

        void restart() override {
            drawn_card = origin->m_game->top_of_deck();
            drawn_card->move_to(pocket_type::discard_pile);

            origin->m_game->add_log("LOG_CHECK_CARD_DRAWN", origin_card, drawn_card);
            if (!origin->m_game->call_event(event_type::on_draw_check_select{ nullptr, shared_from_this() })) {
                resolve();
            }
        }

        card_list get_drawn_cards() const override {
            return {drawn_card};
        }

        card_ptr get_drawing_card() const override {
            return origin_card;
        }

        bool is_positive() const {
            return get_modified_sign(drawn_card).is_red();
        }

        prompt_string redraw_prompt(card_ptr target_card, player_ptr owner) const override {
            if (owner->is_bot() && !is_positive()) {
                return "BOT_DONT_REDRAW_HELENA_ZONTERO";
            }
            return {};
        }
        
        void resolve() override {
            origin->m_game->pop_request();
            if (is_positive()) {
                origin->m_game->add_log("LOG_CARD_HAS_EFFECT", origin_card);
                origin->m_game->queue_request<request_shuffle_roles>(origin_card, origin);
            }
        }
    };

    void equip_helena_zontero::on_enable(card_ptr target_card, player_ptr origin) {
        origin->m_game->queue_request<request_helena_zontero_check>(target_card, origin);
    }
}