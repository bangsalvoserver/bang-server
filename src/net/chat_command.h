#ifndef __CHAT_COMMAND_H__
#define __CHAT_COMMAND_H__

#include <functional>
#include <string>
#include <span>

namespace banggame {

    static constexpr char chat_command_start_char = '/';

    using string_span = std::span<std::string>;

    template<typename ... Args>
    class chat_command {
    private:
        std::move_only_function<void(Args ..., string_span) const> m_fun;

    public:
        template<typename Function>
        chat_command(Function &&fun)
            : m_fun{[fun=std::move(fun)](Args ... extra, string_span args) {
                if constexpr (std::is_invocable_v<Function, Args ..., string_span>) {
                    std::invoke(fun, extra ..., args);
                } else {
                    [&]<size_t ... I>(std::index_sequence<I ...>) {
                        std::invoke(fun, extra ..., (I < args.size() ? args[I] : std::string{}) ...);
                    }(std::make_index_sequence<argument_number_v<std::remove_cvref_t<Function>> - sizeof...(Args)>());
                }
            }} {}

        void operator()(Args ... extra, string_span args) const {
            m_fun(extra ..., args);
        }
    };
}

#endif