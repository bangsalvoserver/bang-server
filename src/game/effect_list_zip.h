#ifndef __EFFECT_LIST_ZIP_H__
#define __EFFECT_LIST_ZIP_H__

#include <ranges>

#include "card_data.h"

namespace banggame {

    using effect_iterator = typename effect_list::const_iterator;
    using target_iterator = typename target_list::const_iterator;

    class effect_zip_sentinel {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = const play_card_target;

        effect_zip_sentinel() = default;
        explicit effect_zip_sentinel(const target_list &targets)
            : m_it(targets.end()) {}

    private:
        target_iterator m_it;

        friend class effect_zip_iterator;
    };

    class effect_zip_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = std::pair<const play_card_target &, const effect_holder &>;

        effect_zip_iterator() = default;
        effect_zip_iterator(const effect_zip_iterator&) = default;
        effect_zip_iterator& operator = (const effect_zip_iterator&) = default;

        effect_zip_iterator(const target_list &targets, const effect_list &effects, const effect_list &optionals)
            : m_targets(&targets)
            , m_effects(&effects)
            , m_optionals(&optionals)
            , m_target_it(targets.begin())
            , m_effect_it(effects.begin())
            , m_effect_end(effects.end()) {}

        value_type operator *() const {
            return {*m_target_it, *m_effect_it};
        }

        effect_zip_iterator& operator++() {
            ++m_target_it;
            ++m_effect_it;
            if (m_effect_it == m_effect_end) {
                m_effect_it = m_optionals->begin();
                m_effect_end = m_optionals->end();
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

        bool operator == (const effect_zip_sentinel &rhs) const noexcept {
            return m_target_it == rhs.m_it;
        }

    private:
        const target_list *m_targets;
        const effect_list *m_effects;
        const effect_list *m_optionals;

        target_iterator m_target_it;
        effect_iterator m_effect_it;
        effect_iterator m_effect_end;
    };

    inline auto zip_card_targets(const target_list &targets, const effect_list &effects, const effect_list &optionals) {
        return std::ranges::subrange(
            effect_zip_iterator(targets, effects, optionals),
            effect_zip_sentinel(targets)
        );
    }

}

#endif