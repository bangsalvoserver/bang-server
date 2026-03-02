#ifndef __UTILS_H__
#define __UTILS_H__

#include <concepts>
#include <string_view>
#include <meta>

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

template<typename Fun> struct argument_number;

template<typename R, typename ... Ts>
struct argument_number<R (*)(Ts...)> : std::integral_constant<size_t, sizeof...(Ts)> {};

template<typename T, typename R, typename ... Ts>
struct argument_number<R (T::*)(Ts...)> : std::integral_constant<size_t, sizeof...(Ts)> {};

template<typename T, typename R, typename ... Ts>
struct argument_number<R (T::*)(Ts...) const> : std::integral_constant<size_t, sizeof...(Ts)> {};

template<typename T> requires requires { &T::operator(); }
struct argument_number<T> : argument_number<decltype(&T::operator())> {};

template<typename Fun>
inline constexpr size_t argument_number_v = argument_number<Fun>::value;

consteval bool has_annotation(std::meta::info info, std::meta::info annotation_type) {
    for (std::meta::info annotation : std::meta::annotations_of(info)) {
        if (std::meta::remove_cv(std::meta::type_of(annotation)) == annotation_type) return true;
    }
    return false;
}

template<typename T>
static constexpr std::string_view type_name = std::meta::identifier_of(^^T);

#endif