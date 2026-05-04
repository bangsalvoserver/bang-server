#ifndef __CARDS_FILTER_ENUMS_H__
#define __CARDS_FILTER_ENUMS_H__

#include "utils/enums.h"

namespace banggame {

    enum class target_player_filter {
        alive,
        dead,
        self,
        notself,
        notsheriff,
        notorigin,
        range_1,
        range_2,
        reachable,
        target_set,
        not_empty,
        not_empty_hand,
        not_empty_table,
        not_empty_cubes,
    };

    enum class target_card_filter {
        target_set,
        selection,
        table,
        hand,
        not_self_hand,
        blue,
        black,
        train,
        blue_or_train,
        hearts,
        diamonds,
        clubs,
        spades,
        origin_card_suit,
        two_to_nine,
        ten_to_ace,
        bang,
        used_bang,
        bangcard,
        not_bangcard,
        missed,
        missedcard,
        not_missedcard,
        beer,
        bronco,
        catbalou_panic,
    };
    
    enum class tag_type {
        // UI - UX tags
        preselect, // when sending this card to respond_cards, the frontend will automatically select it
        button_color, // for button_row cards. 0 = green, 1 = red, 2 = blue
        skip_logs, // bypass log_played_card
        
        // Base game logic tags
        max_hp, // character max hp
        weapon, // weapon card - equip on the left side - also controls weapon "value" for bots
        
        // Bot and prompt logic tags
        penalty, // whether this is a bad equip card
        strong, // controls bot decisions
        
        // Expansion logic tags
        force_allow, // bypass only_base_characters
        last_scenario_card, // move this card to the end of the scenario deck
        buy_cost, // cost for gold rush card
        pardner, // pardner token for this card

        // Modifier logic tags
        card_choice, // grouping id for the card_choice modifier
        bangmod, // can be stacked with other bangmod modifiers
        banglimit, // can be played with Bandolier
        ranged_effect, // can be played with Bell Tower
        play_as_bang, // sets up a "play as BANG!" action - can be played with bangmod modifiers
        
        // Card identifiers
        bangcard, // "real" BANG! card
        missedcard, // "real" Missed! card
        count_as_missed,
        resolve,
        pass_turn,
        pick,
        beer,
        indians, // controls Indian Guide
        catbalou_panic,
        horse,
        jail,
        bronco,
        dynamite,
    };
}

#endif