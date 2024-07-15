#ifndef __GAME_STRING_H__
#define __GAME_STRING_H__

#include "card_fwd.h"
#include "utils/small_pod.h"

namespace banggame {

    static constexpr size_t format_arg_list_max_size = 5;

    class format_arg_list {
    public:
        enum format_arg_type {
            format_number = 0,
            format_card = 1,
            format_player = 2,
        };

        union format_arg {
            int number_value;
            card *card_value;
            player *player_value;
        };

        template<typename ... Ts>
        constexpr format_arg_list(Ts ... values) {
            static_assert(sizeof...(Ts) <= format_arg_list_max_size, "format_arg_list is too big");
            (add(values), ...);
        }

        constexpr std::pair<format_arg_type, format_arg> operator[](std::size_t index) const {
            uint8_t result = types;
            for (std::size_t i=0; i<index; ++i) {
                result /= 3;
            }
            return { static_cast<format_arg_type>(result % 3), args[index] };
        }

        uint8_t size() const {
            return count;
        }

    private:
        std::array<format_arg, format_arg_list_max_size> args{};
        uint8_t count = 0;
        uint8_t types = 0;

        constexpr void add(int value) {
            args[count].number_value = value;
            types += exp3(count) * format_number;
            ++count;
        }

        constexpr void add(card *value) {
            args[count].card_value = value;
            types += exp3(count) * format_card;
            ++count;
        }

        constexpr void add(player *value) {
            args[count].player_value = value;
            types += exp3(count) * format_player;
            ++count;
        }
        
        static constexpr uint8_t exp3(uint8_t exp) {
            uint8_t n = 1;
            while (exp != 0) {
                n *= 3;
                --exp;
            }
            return n;
        }
    };
    
    struct game_string {
        small_string format_str;
        format_arg_list format_args;

        game_string() = default;
    
        game_string(
                std::convertible_to<small_string> auto &&message,
                auto && ... args)
            : format_str(FWD(message))
            , format_args{FWD(args) ...} {}

        explicit operator bool() const {
            return !format_str.empty();
        }
    };
    
    #define MAYBE_RETURN(...) if (auto value_ = __VA_ARGS__) return value_
}

#endif