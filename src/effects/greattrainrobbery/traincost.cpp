#include "traincost.h"

#include "cards/filter_enums.h"

#include "game/game.h"

namespace banggame {

    game_string modifier_traincost::get_error(card *origin_card, player *origin, card *target_card, const effect_context &ctx) {
        if (target_card->pocket != pocket_type::train) {
            return "ERROR_NOT_ALLOWED_WITH_CARD";
        }

        size_t train_index = std::distance(origin->m_game->m_train.begin(), rn::find(origin->m_game->m_train, target_card)) - ctx.train_advance;
        if (train_index > origin->m_game->train_position) {
            return "ERROR_TRAIN_NOT_IN_ANY_STATION";
        }
        if (origin_card->pocket == pocket_type::stations && ctx.playing_card != target_card) {
            return "ERROR_TRAIN_NOT_IN_THIS_STATION";
        }

        return {};
    }

    void modifier_traincost::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.traincost = origin_card;

        if (origin_card->pocket == pocket_type::stations) {
            int station_index = std::distance(origin->m_game->m_stations.begin(), rn::find(origin->m_game->m_stations, origin_card));
            int train_index = ctx.train_advance + origin->m_game->train_position - station_index;
            if (train_index >= 0 && train_index < origin->m_game->m_train.size()) {
                ctx.playing_card = origin->m_game->m_train[train_index];
            }
        }
    }

    bool modifier_locomotive::valid_with_modifier(card *origin_card, player *origin, card *target_card) {
        return target_card->deck == card_deck_type::station
            || target_card->deck != card_deck_type::main_deck && target_card->has_tag(tag_type::traincost);
    }

    bool modifier_locomotive::valid_with_card(card *origin_card, player *origin, card *target_card) {
        return target_card->pocket == pocket_type::train;
    }

    void modifier_locomotive::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.train_advance = 1;
    }

}