#include "traincost.h"

#include "cards/filter_enums.h"
#include "cards/effect_context.h"

#include "game/game.h"

namespace banggame {

    game_string modifier_traincost::get_error(card *origin_card, player *origin, card *target_card, const effect_context &ctx) {
        if (target_card->is_modifier()) {
            return "ERROR_NOT_ALLOWED_WITH_MODIFIER";
        }

        if (target_card->pocket != pocket_type::train) {
            return "ERROR_NOT_ALLOWED_WITH_CARD";
        }

        if (origin_card->pocket == pocket_type::stations) {
            size_t station_index = std::distance(origin->m_game->m_stations.begin(), std::ranges::find(origin->m_game->m_stations, origin_card));
            size_t train_index = std::distance(origin->m_game->m_train.begin(), std::ranges::find(origin->m_game->m_train, target_card));

            if (station_index != ctx.train_advance + origin->m_game->train_position - train_index) {
                return "ERROR_TRAIN_NOT_IN_THIS_STATION";
            }
        }

        return {};
    }

    void modifier_traincost::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.traincost = origin_card;
    }

    bool modifier_locomotive::valid_with_modifier(card *origin_card, player *origin, card *target_card) {
        return target_card->pocket == pocket_type::stations;
    }

    bool modifier_locomotive::valid_with_card(card *origin_card, player *origin, card *target_card) {
        return target_card->pocket == pocket_type::train;
    }

    void modifier_locomotive::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.train_advance = 1;
    }

}