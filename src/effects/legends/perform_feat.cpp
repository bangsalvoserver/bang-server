#include "perform_feat.h"

#include "game/game.h"

namespace banggame {

    bool effect_perform_feat::can_play(card_ptr origin_card, player_ptr origin) {
        int num_feats = 0;
        origin->m_game->call_event(event_type::count_performed_feats{ origin, num_feats });
        return num_feats == 0;
    }

    void effect_perform_feat::on_play(card_ptr origin_card, player_ptr origin) {
        event_card_key key{origin_card, 5};
        origin->m_game->add_listener<event_type::count_performed_feats>(key, [=](player_ptr p, int &num_feats) {
            if (origin == p) {
                ++num_feats;
            }
        });
        origin->m_game->add_listener<event_type::on_turn_end>(key, [=](player_ptr p, bool skipped) {
            if (origin == p) {
                origin->m_game->remove_listeners(key);
            }
        });

        // TODO compiere un'impresa, se origin_card->deck == card_deck_type::feats
        //   se NON sei leggenda - compiere impresa:
        //     -> sposta max 2 fame da character a carta feat
        //     -> track feat compiuta
        //   se SEI leggenda e c'e' almeno un'altra leggenda - rivendicare impresa:
        //     -> pop_request, request_claim_feat: pickable su player leggenda e hp > 1
        //     on_pick:
        //      -> pop_request
        //      -> damage target
        //      -> discard feat (rimuovi fame)
        //      -> draw feat (shuffle se necessario)
        //      -> track feat compiuta
    }
}