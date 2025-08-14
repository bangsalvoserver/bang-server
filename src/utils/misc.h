#ifndef __UTILS_H__
#define __UTILS_H__

#include <concepts>

#define FWD(x) std::forward<decltype(x)>(x)

template<typename ... Ts> struct overloaded : Ts ... { using Ts::operator() ...; };
template<typename ... Ts> overloaded(Ts ...) -> overloaded<Ts ...>;

template<typename T>
class nullable_ref {
private:
    T *m_ptr = nullptr;

public:
    nullable_ref() = default;
    nullable_ref(T &value): m_ptr{&value} {}

    operator T &() const { return *m_ptr; }
    operator const T &() const { return *m_ptr; }
};

template<typename T, typename Function>
struct is_invocable_like;

template<typename T, typename R, typename ... Args>
struct is_invocable_like<T, R(Args ...)> : std::is_invocable_r<R, T, Args ...> {};

template<typename T, typename Function>
inline constexpr bool is_invocable_like_v = is_invocable_like<T, Function>::value;

template<typename T, typename Function>
concept invocable_like = is_invocable_like_v<T, Function>;

#endif