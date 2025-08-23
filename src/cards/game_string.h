#ifndef __GAME_STRING_H__
#define __GAME_STRING_H__

#include "card_fwd.h"

namespace banggame {

    class format_arg_list;

    enum class format_arg_type {
        format_number = 0,
        format_card = 1,
        format_player = 2,
    };

    using small_int_t = int16_t;

    using format_arg_value = std::pair<small_int_t, format_arg_type>;
    
    static constexpr size_t format_arg_list_max_size = 5;

    class format_arg_iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = format_arg_value;
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
        using value_type = format_arg_value;
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

        std::array<small_int_t, format_arg_list_max_size> args{};
        uint8_t count = 0;
        uint8_t types = 0;

        constexpr void add(int value) {
            args[count] = static_cast<small_int_t>(value);
            types += exp3(count) * static_cast<uint8_t>(format_arg_type::format_number);
            ++count;
        }

        constexpr void add(const_card_ptr value) {
            args[count] = static_cast<small_int_t>(get_card_id(value));
            types += exp3(count) * static_cast<uint8_t>(format_arg_type::format_card);
            ++count;
        }

        constexpr void add(const_player_ptr value) {
            args[count] = static_cast<small_int_t>(get_player_id(value));
            types += exp3(count) * static_cast<uint8_t>(format_arg_type::format_player);
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
    
    inline format_arg_value format_arg_iterator::operator *() const {
        uint8_t result = list->types;
        for (std::size_t i = 0; i < index; ++i) {
            result /= 3;
        }
        return { list->args[index], static_cast<format_arg_type>(result % 3) };
    }
    
    struct game_string {
        const char *format_str = nullptr;
        format_arg_list format_args;

        game_string() = default;
    
        template<size_t N, typename ... Ts>
        game_string(const char (&message)[N], Ts && ... args)
            : format_str(message)
            , format_args{std::forward<Ts>(args) ...} {}

        explicit operator bool() const {
            return format_str != nullptr;
        }
    };

    struct prompt_string {
        game_string message;
        int priority = 0;

        prompt_string() = default;

        prompt_string(game_string message)
            : message{message} {}

        template<size_t N, typename ... Ts>
        prompt_string(const char (&message)[N], Ts && ... args)
            : message{message, std::forward<Ts>(args) ...} {}
        
        template<size_t N, typename ... Ts>
        prompt_string(int priority, const char (&message)[N], Ts && ... args)
            : message{message, std::forward<Ts>(args) ...}
            , priority{priority} {}

        explicit operator bool() const {
            return bool(message);
        }
    };
    
    #define MAYBE_RETURN(...) if (auto value_ = __VA_ARGS__) return value_
}

#endif