#ifndef __GAME_STRING_H__
#define __GAME_STRING_H__

#include "utils/small_string.h"
#include "card_fwd.h"

namespace banggame {

    static constexpr size_t format_arg_list_max_size = 5;

    namespace game_string_args {
        struct integer {
            struct transparent{};
            int value;
        };

        struct card {
            struct transparent{};
            utils::nullable<banggame::const_card_ptr> value;
        };

        struct player {
            struct transparent{};
            utils::nullable<banggame::const_player_ptr> value;
        };
    }

    using format_arg_variant = std::variant<
        game_string_args::integer,
        game_string_args::card,
        game_string_args::player
    >;

    class format_arg_list;

    class format_arg_iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = format_arg_variant;
        using pointer = value_type *;
        using reference = value_type;
    
    private:
        const format_arg_list *list;
        size_t index;
    
    public:
        format_arg_iterator() = default;

        format_arg_iterator(const format_arg_list *list, size_t index)
            : list{list}, index{index} {}
        
        bool operator == (const format_arg_iterator &other) const = default;

        format_arg_iterator &operator ++ () {
            ++index;
            return *this;
        }

        format_arg_iterator operator ++ (int) {
            auto copy = *this;
            ++index;
            return copy;
        }

        format_arg_iterator &operator --() {
            --index;
            return *this;
        }

        format_arg_iterator operator -- (int) {
            auto copy = *this;
            --index;
            return copy;
        }

        value_type operator *() const;
    };

    class format_arg_list {
    public:
        using value_type = format_arg_variant;
        using iterator = format_arg_iterator;
        using const_iterator = format_arg_iterator;
        using reverse_iterator = format_arg_iterator;

        template<typename ... Ts>
        constexpr format_arg_list(Ts ... values) {
            static_assert(sizeof...(Ts) <= format_arg_list_max_size, "format_arg_list is too big");
            (add(values), ...);
        }

        size_t size() const {
            return static_cast<size_t>(count);
        }

        format_arg_iterator begin() const {
            return {this, 0};
        }

        format_arg_iterator end() const {
            return {this, size()};
        }

    private:
        friend class format_arg_iterator;

        enum format_arg_type {
            format_number = 0,
            format_card = 1,
            format_player = 2,
        };

        union format_arg {
            int number_value;
            const_card_ptr card_value;
            const_player_ptr player_value;
        };

        std::array<format_arg, format_arg_list_max_size> args{};
        uint8_t count = 0;
        uint8_t types = 0;

        constexpr void add(int value) {
            args[count].number_value = value;
            types += exp3(count) * format_number;
            ++count;
        }

        constexpr void add(const_card_ptr value) {
            args[count].card_value = value;
            types += exp3(count) * format_card;
            ++count;
        }

        constexpr void add(const_player_ptr value) {
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
    
    inline format_arg_variant format_arg_iterator::operator *() const {
        uint8_t result = list->types;
        for (std::size_t i = 0; i < index; ++i) {
            result /= 3;
        }
        auto type = static_cast<format_arg_list::format_arg_type>(result % 3);
        auto arg = list->args[index];
        
        switch (type) {
        case banggame::format_arg_list::format_number:
            return game_string_args::integer{ arg.number_value};
        case banggame::format_arg_list::format_card:
            return game_string_args::card{ arg.card_value};
        case banggame::format_arg_list::format_player:
            return game_string_args::player{ arg.player_value};
        default:
            throw std::runtime_error("invalid format_arg");
        }
    }
    
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

    enum class prompt_type {
        normal,
        priority
    };

    struct prompt_string {
        game_string message;
        prompt_type type = prompt_type::normal;

        prompt_string() = default;

        prompt_string(game_string message)
            : message{message} {}

        prompt_string(
                std::convertible_to<small_string> auto &&message,
                auto && ... args)
            : message{message, FWD(args) ...} {}
        
        prompt_string(prompt_type type,
                std::convertible_to<small_string> auto &&message,
                auto && ... args)
            : message{message, FWD(args) ...}
            , type{type} {}

        explicit operator bool() const {
            return bool(message);
        }
    };
    
    #define MAYBE_RETURN(...) if (auto value_ = __VA_ARGS__) return value_
}

#endif