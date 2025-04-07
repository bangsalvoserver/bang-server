#include "sgt_blaze.h"

#include "game/filters.h"
#include "cards/filter_enums.h"

#include "game/game_table.h"

#include "effects/base/pick.h"
#include "effects/base/resolve.h"

#include "ruleset.h"

namespace banggame {

    game_string modifier_sgt_blaze::get_error(card_ptr origin_card, player_ptr origin, card_ptr playing_card, const effect_context &ctx) {
        for (const effect_holder &effect : playing_card->get_effect_list(origin->m_game->pending_requests())) {
            if (effect.target == TARGET_TYPE(players) || effect.target == TARGET_TYPE(card_per_player)) {
                if (ctx.skipped_player && check_player_filter(playing_card, origin, effect.player_filter, ctx.skipped_player, ctx)) {
                    return {"ERROR_CANNOT_SKIP_PLAYER", ctx.skipped_player};
                } else {
                    return {};
                }
            }
        }
        return {"ERROR_NO_PLAYERS_TARGET", origin_card, playing_card};
    }

    void effect_skip_player::add_context(card_ptr origin_card, player_ptr origin, player_ptr target, effect_context &ctx) {
        ctx.skipped_player = target;
    }

    void effect_skip_player::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) {
        origin->m_game->add_log("LOG_SKIP_PLAYER", origin_card, origin, target, ctx.playing_card);
    }

    struct request_sgt_blaze : request_resolvable, interface_picking_player {
        request_sgt_blaze(card_ptr origin_card, player_ptr target, shared_locomotive_context ctx)
            : request_resolvable(origin_card, nullptr, target)
            , ctx(std::move(ctx)) {}
        
        shared_locomotive_context ctx;

        card_list get_highlights(player_ptr owner) const override {
            return {target->m_game->m_train.front()};
        }

        void on_update() override {
            if (update_count == 0) {
                ctx->skipped_player = nullptr;
            }
        }

        resolve_type get_resolve_type() const override {
            return resolve_type::dismiss;
        }

        void on_resolve() override {
            target->m_game->pop_request();
        }

        bool can_pick(const_player_ptr target_player) const override {
            return true;
        }

        void on_pick(player_ptr target_player) override {
            ctx->skipped_player = target_player;
            target->m_game->add_log("LOG_SKIP_PLAYER", origin_card, target, target_player, target->m_game->m_train.front());
            target->m_game->pop_request();
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_SGT_BLAZE_LOCOMOTIVE", origin_card};
            } else {
                return {"STATUS_SGT_BLAZE_LOCOMOTIVE_OTH", origin_card, target};
            }
        }
    };

    void equip_sgt_blaze::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_locomotive_effect>({origin_card, 1}, [=](player_ptr target, shared_locomotive_context ctx) {
            if (origin == target) {
                origin->m_game->queue_request<request_sgt_blaze>(origin_card, origin, ctx);
            }
        });
    }

}