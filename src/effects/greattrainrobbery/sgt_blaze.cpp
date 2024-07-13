#include "sgt_blaze.h"

#include "game/filters.h"
#include "cards/filter_enums.h"

#include "game/game.h"

#include "effects/base/resolve.h"

#include "ruleset.h"

namespace banggame {

    game_string modifier_sgt_blaze::get_error(card *origin_card, player *origin, card *playing_card, const effect_context &ctx) {
        const auto &effects = playing_card->get_effect_list(origin->m_game->pending_requests());
        if (auto it = rn::find(effects, TARGET_TYPE(players), &effect_holder::target); it != effects.end()) {
            if (ctx.skipped_player && filters::check_player_filter(origin, it->player_filter, ctx.skipped_player, ctx)) {
                return {"ERROR_CANNOT_SKIP_PLAYER", ctx.skipped_player};
            }
        } else if (playing_card->has_tag(tag_type::multi_target)) {
            if (ctx.skipped_player == origin) {
                return {"ERROR_CANNOT_SKIP_PLAYER", origin};
            }
        } else {
            return {"ERROR_NO_PLAYERS_TARGET", origin_card, playing_card};
        }
        return {};
    }

    void effect_skip_player::add_context(card *origin_card, player *origin, player *target, effect_context &ctx) {
        ctx.skipped_player = target;
    }

    void effect_skip_player::on_play(card *origin_card, player *origin, player *target, const effect_context &ctx) {
        origin->m_game->add_log("LOG_SKIP_PLAYER", origin_card, origin, target, ctx.playing_card);
    }

    struct request_sgt_blaze : request_resolvable {
        request_sgt_blaze(card *origin_card, player *target, shared_effect_context ctx)
            : request_resolvable(origin_card, nullptr, target)
            , ctx(std::move(ctx)) {}
        
        shared_effect_context ctx;

        std::vector<card *> get_highlights() const override {
            return {target->m_game->m_train.front()};
        }

        void on_update() override {
            if (!live) {
                ctx->skipped_player = nullptr;
            }
        }

        void on_resolve() override {
            target->m_game->pop_request();
        }

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_SGT_BLAZE_LOCOMOTIVE", origin_card};
            } else {
                return {"STATUS_SGT_BLAZE_LOCOMOTIVE_OTH", origin_card, target};
            }
        }
    };

    void equip_sgt_blaze::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_locomotive_effect>({origin_card, 1}, [=](player *target, shared_effect_context ctx) {
            if (origin == target) {
                origin->m_game->queue_request<request_sgt_blaze>(origin_card, origin, ctx);
            }
        });
    }

    game_string effect_skip_player_locomotive::get_error(card *origin_card, player *origin, player *target) {
        if (origin->m_game->top_request<request_sgt_blaze>(origin)) {
            return {};
        }
        return "ERROR_INVALID_ACTION";
    }

    void effect_skip_player_locomotive::on_play(card *origin_card, player *origin, player *target) {
        auto req = origin->m_game->top_request<request_sgt_blaze>();
        req->ctx->skipped_player = target;
        origin->m_game->add_log("LOG_SKIP_PLAYER", origin_card, origin, target, origin->m_game->m_train.front());
        origin->m_game->pop_request();
    }

}