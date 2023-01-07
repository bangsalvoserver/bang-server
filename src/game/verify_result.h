#ifndef __VERIFY_RESULT_H__
#define __VERIFY_RESULT_H__

#include "game_string.h"

#include <vector>
#include <memory>

namespace banggame {

    struct verify_modifier {
        virtual ~verify_modifier() = default;
    };

    class verify_result {
    private:
        std::vector<game_string> errors;
        std::vector<std::unique_ptr<verify_modifier>> modifiers;
    
    public:
        verify_result() = default;
        
        template<typename ... Ts>
        verify_result(Ts && ... args) {
            add_error(game_string{FWD(args) ...});
        }

        template<std::derived_from<verify_modifier> T, typename ... Ts>
        verify_result(std::in_place_type_t<T>, Ts && ... args) {
            modifiers.push_back(std::make_unique<T>(FWD(args) ...));
        }

        void add_error(game_string error) {
            if (error) {
                errors.emplace_back(std::move(error));
            }
        }

        void add(verify_result &&other) {
            std::ranges::move(other.errors, std::back_inserter(errors));
            std::ranges::move(other.modifiers, std::back_inserter(modifiers));
        }

        operator game_string () const {
            if (errors.empty()) {
                return {};
            } else {
                return errors.front();
            }
        }

        operator bool () const {
            return !errors.empty();
        }
    };

}

#endif