#include "expansion_set.h"

#include "cards/bang_cards.h"

namespace banggame {

    ruleset_ptr expansion_set_iterator::operator *() const {
        return (bang_cards.expansions.begin() + m_index)->second.expansion;
    }
    
    uint64_t get_expansion_bit(ruleset_ptr value) {
        // This is O(N), which is fine
        uint64_t result = 1;
        for (const auto &[name, expansion] : bang_cards.expansions) {
            if (expansion.expansion == value) {
                return result;
            }
            result <<= 1;
        }
        // Should never happen
        return 0;
    }
    
    ruleset_ptr get_expansion_by_name(std::string_view name) {
        auto it = bang_cards.expansions.find(name);
        if (it != bang_cards.expansions.end()) {
            return it->second.expansion;
        }
        return nullptr;
    }

    std::string_view get_expansion_name(ruleset_ptr value) {
        return value->name;
    }
    
    bool validate_expansions(const expansion_set &expansions) {
        for (ruleset_ptr ruleset : expansions) {
            if (!ruleset->is_valid_with(expansions)) {
                return false;
            }
        }
        return true;
    }
}