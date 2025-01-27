#ifndef __UTILS_FUNCTION_REF_H__
#define __UTILS_FUNCTION_REF_H__

#include <functional>
#include <utility>
#include <cassert>

namespace std23
{

template<auto V> struct nontype_t // freestanding
{
    explicit nontype_t() = default;
};

template<auto V> inline constexpr nontype_t<V> nontype{}; // freestanding

using std::in_place_type;
using std::in_place_type_t;
using std::initializer_list;
using std::nullptr_t;

template<class R, class F, class... Args>
requires std::is_invocable_r_v<R, F, Args...>
constexpr R invoke_r(F &&f, Args &&...args) // freestanding
    noexcept(std::is_nothrow_invocable_r_v<R, F, Args...>)
{
    if constexpr (std::is_void_v<R>)
        std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    else
        return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}

// See also: https://www.agner.org/optimize/calling_conventions.pdf
template<class T>
inline constexpr auto _select_param_type = []
{
    if constexpr (std::is_trivially_copyable_v<T>)
        return std::type_identity<T>();
    else
        return std::add_rvalue_reference<T>();
};

template<class T>
using _param_t = std::invoke_result_t<decltype(_select_param_type<T>)>::type;

template<class T, class Self>
inline constexpr bool _is_not_self =
    not std::is_same_v<std::remove_cvref_t<T>, Self>;

template<class T, template<class...> class>
inline constexpr bool _looks_nullable_to_impl = std::is_member_pointer_v<T>;

template<class F, template<class...> class Self>
inline constexpr bool _looks_nullable_to_impl<F *, Self> =
    std::is_function_v<F>;

template<class... S, template<class...> class Self>
inline constexpr bool _looks_nullable_to_impl<Self<S...>, Self> = true;

template<class S, template<class...> class Self>
inline constexpr bool _looks_nullable_to =
    _looks_nullable_to_impl<std::remove_cvref_t<S>, Self>;

template<class T> inline constexpr bool _is_not_nontype_t = true;
template<auto f> inline constexpr bool _is_not_nontype_t<nontype_t<f>> = false;

template<class T> struct _adapt_signature;

template<class F> requires std::is_function_v<F>
struct _adapt_signature<F *>
{
    using type = F;
};

template<class Fp> using _adapt_signature_t = _adapt_signature<Fp>::type;

template<class S> struct _not_qualifying_this
{};

template<class R, class... Args> struct _not_qualifying_this<R(Args...)>
{
    using type = R(Args...);
};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) noexcept>
{
    using type = R(Args...) noexcept;
};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) const> : _not_qualifying_this<R(Args...)>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) volatile>
    : _not_qualifying_this<R(Args...)>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) const volatile>
    : _not_qualifying_this<R(Args...)>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) &> : _not_qualifying_this<R(Args...)>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) const &>
    : _not_qualifying_this<R(Args...)>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) volatile &>
    : _not_qualifying_this<R(Args...)>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) const volatile &>
    : _not_qualifying_this<R(Args...)>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) &&> : _not_qualifying_this<R(Args...)>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) const &&>
    : _not_qualifying_this<R(Args...)>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) volatile &&>
    : _not_qualifying_this<R(Args...)>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) const volatile &&>
    : _not_qualifying_this<R(Args...)>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) const noexcept>
    : _not_qualifying_this<R(Args...) noexcept>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) volatile noexcept>
    : _not_qualifying_this<R(Args...) noexcept>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) const volatile noexcept>
    : _not_qualifying_this<R(Args...) noexcept>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) & noexcept>
    : _not_qualifying_this<R(Args...) noexcept>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) const & noexcept>
    : _not_qualifying_this<R(Args...) noexcept>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) volatile & noexcept>
    : _not_qualifying_this<R(Args...) noexcept>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) const volatile & noexcept>
    : _not_qualifying_this<R(Args...) noexcept>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) && noexcept>
    : _not_qualifying_this<R(Args...) noexcept>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) const && noexcept>
    : _not_qualifying_this<R(Args...) noexcept>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) volatile && noexcept>
    : _not_qualifying_this<R(Args...) noexcept>
{};

template<class R, class... Args>
struct _not_qualifying_this<R(Args...) const volatile && noexcept>
    : _not_qualifying_this<R(Args...) noexcept>
{};

template<class F, class T> struct _drop_first_arg_to_invoke;

template<class T, class R, class G, class... Args>
struct _drop_first_arg_to_invoke<R (*)(G, Args...), T>
{
    using type = R(Args...);
};

template<class T, class R, class G, class... Args>
struct _drop_first_arg_to_invoke<R (*)(G, Args...) noexcept, T>
{
    using type = R(Args...) noexcept;
};

template<class T, class M, class G> requires std::is_object_v<M>
struct _drop_first_arg_to_invoke<M G::*, T>
{
    using type = std::invoke_result_t<M G::*, T>();
};

template<class T, class M, class G> requires std::is_function_v<M>
struct _drop_first_arg_to_invoke<M G::*, T> : _not_qualifying_this<M>
{};

template<class F, class T>
using _drop_first_arg_to_invoke_t = _drop_first_arg_to_invoke<F, T>::type;

template<class Sig> struct _qual_fn_sig;

template<class R, class... Args> struct _qual_fn_sig<R(Args...)>
{
    using function = R(Args...);
    static constexpr bool is_noexcept = false;

    template<class... T>
    static constexpr bool is_invocable_using =
        std::is_invocable_r_v<R, T..., Args...>;

    template<class T> using cv = T;
};

template<class R, class... Args> struct _qual_fn_sig<R(Args...) noexcept>
{
    using function = R(Args...);
    static constexpr bool is_noexcept = true;

    template<class... T>
    static constexpr bool is_invocable_using =
        std::is_nothrow_invocable_r_v<R, T..., Args...>;

    template<class T> using cv = T;
};

template<class R, class... Args>
struct _qual_fn_sig<R(Args...) const> : _qual_fn_sig<R(Args...)>
{
    template<class T> using cv = T const;
};

template<class R, class... Args>
struct _qual_fn_sig<R(Args...) const noexcept>
    : _qual_fn_sig<R(Args...) noexcept>
{
    template<class T> using cv = T const;
};

struct _function_ref_base
{
    union storage
    {
        void *p_ = nullptr;
        void const *cp_;
        void (*fp_)();

        constexpr storage() noexcept = default;

        template<class T> requires std::is_object_v<T>
        constexpr explicit storage(T *p) noexcept : p_(p)
        {}

        template<class T> requires std::is_object_v<T>
        constexpr explicit storage(T const *p) noexcept : cp_(p)
        {}

        template<class T> requires std::is_function_v<T>
        constexpr explicit storage(T *p) noexcept
            : fp_(reinterpret_cast<decltype(fp_)>(p))
        {}
    };

    template<class T> constexpr static auto get(storage obj)
    {
        if constexpr (std::is_const_v<T>)
            return static_cast<T *>(obj.cp_);
        else if constexpr (std::is_object_v<T>)
            return static_cast<T *>(obj.p_);
        else
            return reinterpret_cast<T *>(obj.fp_);
    }
};

template<class Sig, class = typename _qual_fn_sig<Sig>::function>
class function_ref; // freestanding

template<class Sig, class R, class... Args>
class function_ref<Sig, R(Args...)> // freestanding
    : _function_ref_base
{
    using signature = _qual_fn_sig<Sig>;

    template<class T> using cv = signature::template cv<T>;
    template<class T> using cvref = cv<T> &;
    static constexpr bool noex = signature::is_noexcept;

    template<class... T>
    static constexpr bool is_invocable_using =
        signature::template is_invocable_using<T...>;

    typedef R fwd_t(storage, _param_t<Args>...) noexcept(noex);
    fwd_t *fptr_ = nullptr;
    storage obj_;

  public:
    template<class F>
    function_ref(F *f) noexcept
        requires std::is_function_v<F> and is_invocable_using<F>
        : fptr_(
              [](storage fn_, _param_t<Args>... args) noexcept(noex) -> R
              {
                  if constexpr (std::is_void_v<R>)
                      get<F>(fn_)(static_cast<decltype(args)>(args)...);
                  else
                      return get<F>(fn_)(static_cast<decltype(args)>(args)...);
              }),
          obj_(f)
    {
        assert(f != nullptr && "must reference a function");
    }

    template<class F, class T = std::remove_reference_t<F>>
    constexpr function_ref(F &&f) noexcept
        requires(_is_not_self<F, function_ref> and
                 not std::is_member_pointer_v<T> and
                 is_invocable_using<cvref<T>>)
        : fptr_(
              [](storage fn_, _param_t<Args>... args) noexcept(noex) -> R
              {
                  cvref<T> obj = *get<T>(fn_);
                  if constexpr (std::is_void_v<R>)
                      obj(static_cast<decltype(args)>(args)...);
                  else
                      return obj(static_cast<decltype(args)>(args)...);
              }),
          obj_(std::addressof(f))
    {}

    template<class T>
    function_ref &operator=(T)
        requires(_is_not_self<T, function_ref> and not std::is_pointer_v<T> and
                 _is_not_nontype_t<T>)
        = delete;

    template<auto f>
    constexpr function_ref(nontype_t<f>) noexcept
        requires is_invocable_using<decltype(f)>
        : fptr_(
              [](storage, _param_t<Args>... args) noexcept(noex) -> R {
                  return std23::invoke_r<R>(
                      f, static_cast<decltype(args)>(args)...);
              })
    {
        using F = decltype(f);
        if constexpr (std::is_pointer_v<F> or std::is_member_pointer_v<F>)
            static_assert(f != nullptr, "NTTP callable must be usable");
    }

    template<auto f, class U, class T = std::remove_reference_t<U>>
    constexpr function_ref(nontype_t<f>, U &&obj) noexcept
        requires(not std::is_rvalue_reference_v<U &&> and
                 is_invocable_using<decltype(f), cvref<T>>)
        : fptr_(
              [](storage this_, _param_t<Args>... args) noexcept(noex) -> R
              {
                  cvref<T> obj = *get<T>(this_);
                  return std23::invoke_r<R>(
                      f, obj, static_cast<decltype(args)>(args)...);
              }),
          obj_(std::addressof(obj))
    {
        using F = decltype(f);
        if constexpr (std::is_pointer_v<F> or std::is_member_pointer_v<F>)
            static_assert(f != nullptr, "NTTP callable must be usable");
    }

    template<auto f, class T>
    constexpr function_ref(nontype_t<f>, cv<T> *obj) noexcept
        requires is_invocable_using<decltype(f), decltype(obj)>
        : fptr_(
              [](storage this_, _param_t<Args>... args) noexcept(noex) -> R
              {
                  return std23::invoke_r<R>(
                      f, get<cv<T>>(this_),
                      static_cast<decltype(args)>(args)...);
              }),
          obj_(obj)
    {
        using F = decltype(f);
        if constexpr (std::is_pointer_v<F> or std::is_member_pointer_v<F>)
            static_assert(f != nullptr, "NTTP callable must be usable");

        if constexpr (std::is_member_pointer_v<F>)
            assert(obj != nullptr && "must reference an object");
    }

    constexpr R operator()(Args... args) const noexcept(noex)
    {
        return fptr_(obj_, std::forward<Args>(args)...);
    }
};

template<class F> requires std::is_function_v<F>
function_ref(F *) -> function_ref<F>;

template<auto V>
function_ref(nontype_t<V>) -> function_ref<_adapt_signature_t<decltype(V)>>;

template<auto V, class T>
function_ref(nontype_t<V>, T &&)
    -> function_ref<_drop_first_arg_to_invoke_t<decltype(V), T &>>;

} // namespace std23

#endif