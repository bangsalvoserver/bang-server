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

        effect_zip_iterator(const target_list &targets, const effect_list &effects, const effect_list &optionals)
            : m_target_it(targets.begin())
            , m_effect_it(effects.begin()), m_effect_end(effects.end())
            , m_optionals_begin(optionals.begin()), m_optionals_end(optionals.end()) {}

        value_type operator *() const {
            return {*m_target_it, *m_effect_it};
        }

        effect_zip_iterator& operator++() {
            ++m_target_it;
            ++m_effect_it;
            if (m_effect_it == m_effect_end) {
                m_effect_it = m_optionals_begin;
                m_effect_end = m_optionals_end;
            }
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

        effect_iterator m_optionals_begin;
        effect_iterator m_optionals_end;
    };

    inline auto zip_card_targets(const target_list &targets, const effect_list &effects, const effect_list &optionals) {
        return rn::subrange(
            effect_zip_iterator(targets, effects, optionals),
            targets.end()
        );
    }

    inline auto zip_card_targets(const target_list &targets, card_data *origin_card, bool is_response) {
        return zip_card_targets(targets, origin_card->get_effect_list(is_response), origin_card->optionals);
    }

}

#endif