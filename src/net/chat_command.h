#ifndef __CHAT_COMMAND_H__
#define __CHAT_COMMAND_H__

#include <functional>
#include <string>
#include <span>

namespace banggame {

    struct game_lobby;

    using string_span = std::span<std::string>;

    class chat_command {
    private:
        std::move_only_function<void(game_lobby &lobby, int user_id, string_span args) const> m_fun;

    public:
        static constexpr char start_char = '/';

        template<typename Function>
        chat_command(Function &&fun)
            : m_fun{[fun=std::move(fun)](game_lobby &lobby, int user_id, string_span args) {
                [&]<size_t ... I>(std::index_sequence<I ...>) {
                    std::invoke(fun, lobby, user_id, (I < args.size() ? args[I] : std::string{}) ...);
                }(std::make_index_sequence<argument_number_v<std::remove_cvref_t<Function>> - 2>());
            }} {}

        void operator()(game_lobby &lobby, int user_id, string_span args) const {
            m_fun(lobby, user_id, args);
        }
    };
}

#endif