#ifndef __ZIP_CARD_TARGETS_H__
#define __ZIP_CARD_TARGETS_H__

#include "cards/card_defs.h"

namespace banggame {

    class effect_zip_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = effect_target_pair;

        using target_iterator = typename target_list::const_iterator;
        using effect_iterator = typename effect_list::const_iterator;

        effect_zip_iterator() = default;
        effect_zip_iterator(const effect_zip_iterator&) = default;
        effect_zip_iterator& operator = (const effect_zip_iterator&) = default;

        effect_zip_iterator(const target_list &targets, const effect_list &effects)
            : m_target_it(targets.begin())
            , m_effect_it(effects.begin()), m_effect_end(effects.end()) {}

        value_type operator *() const {
            return {*m_target_it, *m_effect_it};
        }

        effect_zip_iterator& operator++() {
            ++m_target_it;
            ++m_effect_it;
            return *this;
        }

        effect_zip_iterator operator++(int) {
            effect_zip_iterator tmp{*this};
            ++(*this);
            return tmp;
        }

        bool operator == (const effect_zip_iterator &rhs) const noexcept {
            return m_target_it == rhs.m_target_it;
        }

        bool operator == (target_iterator rhs) const noexcept {
            return m_target_it == rhs;
        }

    private:
        target_iterator m_target_it;

        effect_iterator m_effect_it;
        effect_iterator m_effect_end;
    };

    inline auto zip_card_targets(const target_list &targets, const effect_list &effects) {
        return rn::subrange(effect_zip_iterator(targets, effects), targets.end());
    }

    inline auto zip_card_targets(const target_list &targets, card_data *origin_card, bool is_response) {
        return zip_card_targets(targets, origin_card->get_effect_list(is_response));
    }

}

#endif