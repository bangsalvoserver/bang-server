#ifndef __EFFECT_LIST_ZIP_H__
#define __EFFECT_LIST_ZIP_H__

#include "card_data.h"
#include "utils/generator.h"

namespace banggame {

    inline util::generator<std::pair<const play_card_target &, const effect_holder &>> zip_card_targets(const target_list &targets, const effect_list &effects, const effect_list &optionals) {
        auto target_it = targets.begin();

        for (auto effect_it = effects.begin(); target_it != targets.end() && effect_it != effects.end(); ++target_it, ++effect_it) {
            co_yield {*target_it, *effect_it};
        }

        for (auto effect_it = optionals.begin(); target_it != targets.end(); ++target_it, ++effect_it) {
            if (effect_it == optionals.end()) {
                effect_it = optionals.begin();
            }
            co_yield {*target_it, *effect_it};
        }
    }

}

#endif