#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef USE_STD_RANGES

#include <ranges>

namespace rn = std::ranges;
namespace rv = std::views;

#else

#include <range/v3/all.hpp>

namespace rn = ranges;
namespace rv = ranges::views;

#endif

#define FWD(x) std::forward<decltype(x)>(x)

template<typename T> class not_null;
template<typename T> class not_null<T *> {
public:
    not_null() = default;
    not_null(std::nullptr_t) { check(); }
    not_null(T *value) : value(value) { check(); }

    operator T *() { check(); return value; }
    operator T *() const { check(); return value; }

    T *get() { check(); return value; }
    T *get() const { check(); return value; }

    T &operator *() { check(); return *value; }
    T &operator *() const { check(); return *value; }

    T *operator -> () { check(); return value; }
    T *operator -> () const { check(); return value; }

private:
    void check() const {
        if (!value) throw std::runtime_error("value can not be null");
    }

private:
    T *value = nullptr;
};

template<typename T> not_null(T *) -> not_null<T *>;


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

#endif