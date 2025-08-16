#ifndef __RANGES_CONCAT_H__
#define __RANGES_CONCAT_H__

#include <ranges>
#include <tuple>
#include <variant>
#include <cassert>

namespace std::ranges {

  namespace detail {

    template<typename Range>
    concept simple_view = view<Range> && range<const Range>
      && same_as<iterator_t<Range>, iterator_t<const Range>>
      && same_as<sentinel_t<Range>, sentinel_t<const Range>>;
    
    template<bool IsConst, typename T>
    using maybe_const_t = conditional_t<IsConst, const T, T>;

    template<bool IsConst, typename... Vs>
    concept all_random_access = (random_access_range<maybe_const_t<IsConst, Vs>> && ...);

    template<bool IsConst, typename... Vs>
    concept all_bidirectional = (bidirectional_range<maybe_const_t<IsConst, Vs>> && ...);

    template<bool IsConst, typename... Vs>
    concept all_forward = (forward_range<maybe_const_t<IsConst, Vs>> && ...);

    template<typename... Rs>
    using concat_reference_t = common_reference_t<range_reference_t<Rs>...>;

    template<typename... Rs>
    using concat_value_t = common_type_t<range_value_t<Rs>...>;

    template<typename... Rs>
    using concat_rvalue_reference_t = common_reference_t<range_rvalue_reference_t<Rs>...>;

    template<typename Ref, typename RRef, typename It>
    concept concat_indirectly_readable_impl = requires(const It it) {
      { *it } -> convertible_to<Ref>;
      { ranges::iter_move(it) } -> convertible_to<RRef>;
    };

    template<typename... Rs>
    concept concat_indirectly_readable
      = common_reference_with<concat_reference_t<Rs...>&&, concat_value_t<Rs...>&>
        && common_reference_with<concat_reference_t<Rs...>&&,
              concat_rvalue_reference_t<Rs...>&&>
        && common_reference_with<concat_rvalue_reference_t<Rs...>&&,
              concat_value_t<Rs...> const&>
        && (concat_indirectly_readable_impl<concat_reference_t<Rs...>,
                concat_rvalue_reference_t<Rs...>,
                iterator_t<Rs>>
            && ...);

    template<typename... Rs>
    concept concatable = requires {
      typename concat_reference_t<Rs...>;
      typename concat_value_t<Rs...>;
      typename concat_rvalue_reference_t<Rs...>;
    } && concat_indirectly_readable<Rs...>;

    template<bool IsConst, typename Range, typename... Rs>
    struct all_but_last_common {
      static inline constexpr bool value = requires {
        requires (common_range<maybe_const_t<IsConst, Range>>
          && all_but_last_common<IsConst, Rs...>::value);
      };
    };

    template<bool IsConst, typename Range>
    struct all_but_last_common<IsConst, Range> {
      static inline constexpr bool value = true;
    };

    template<bool IsConst, typename... Rs>
    concept concat_is_random_access = all_random_access<IsConst, Rs...>
      && all_but_last_common<IsConst, Rs...>::value;

    template<bool IsConst, typename... Rs>
    concept concat_is_bidirectional = all_bidirectional<IsConst, Rs...>
      && all_but_last_common<IsConst, Rs...>::value;

    template<typename Range, typename... Rs>
    struct all_but_first_sized {
      static inline constexpr bool value = (sized_range<Rs> && ...);
    };

    template<bool IsConst, typename... Vs>
    class concat_view_iter_cat { };

    template<bool IsConst, typename... Vs>
      requires detail::all_forward<IsConst, Vs...>
    class concat_view_iter_cat<IsConst, Vs...> {
    private:
      static auto get_iterator_category() {
        if constexpr (!is_reference_v<concat_reference_t<maybe_const_t<IsConst, Vs>...>>)
          return input_iterator_tag{};
        else
          return []<typename... Cats>(Cats...) {
            if constexpr ((derived_from<Cats, random_access_iterator_tag> && ...)
                && concat_is_random_access<IsConst, Vs...>)
              return random_access_iterator_tag{};
            else if constexpr ((derived_from<Cats, bidirectional_iterator_tag> && ...)
                && concat_is_bidirectional<IsConst, Vs...>)
              return bidirectional_iterator_tag{};
            else if constexpr ((derived_from<Cats, forward_iterator_tag> && ...))
              return forward_iterator_tag{};
            else
              return input_iterator_tag{};
          }(typename iterator_traits<iterator_t<maybe_const_t<IsConst, Vs>>>::iterator_category{}...);
      }

    public:
      using iterator_category = decltype(get_iterator_category());
    };
  }

  template<input_range... Vs>
    requires (view<Vs> && ...) && (sizeof...(Vs) > 0) && detail::concatable<Vs...>
  class concat_view : public view_interface<concat_view<Vs...>> {
    tuple<Vs...> m_views;

    template<bool IsConst>
    class iterator : public detail::concat_view_iter_cat<IsConst, Vs...> {
      static auto get_iterator_concept() {
        if constexpr (detail::concat_is_random_access<IsConst, Vs...>)
          return random_access_iterator_tag{};
        else if constexpr (detail::concat_is_bidirectional<IsConst, Vs...>)
          return bidirectional_iterator_tag{};
        else if constexpr (detail::all_forward<IsConst, Vs...>)
          return forward_iterator_tag{};
        else
          return input_iterator_tag{};
      }

      friend concat_view<Vs...>;
      friend iterator<!IsConst>;

    public:
      using iterator_concept = decltype(get_iterator_concept());
      using value_type = detail::concat_value_t<detail::maybe_const_t<IsConst, Vs>...>;
      using difference_type = common_type_t<range_difference_t<detail::maybe_const_t<IsConst, Vs>>...>;

    private:
      using base_iter = variant<iterator_t<detail::maybe_const_t<IsConst, Vs>>...>;

      detail::maybe_const_t<IsConst, tuple<Vs...>>* m_views = nullptr;
      base_iter m_it;

      template<size_t N>
      constexpr void satisfy() {
        if constexpr (N < (sizeof...(Vs) - 1)) {
          if (std::get<N>(m_it) == ranges::end(std::get<N>(*m_views))) {
            m_it.template emplace<N + 1>(ranges::begin(std::get<N + 1>(*m_views)));
            satisfy<N + 1>();
          }
        }
      }

      template<size_t N>
      constexpr void prev() {
        if constexpr (N == 0) {
          --std::get<0>(m_it);
        } else if (std::get<N>(m_it) == ranges::begin(std::get<N>(*m_views))) {
          m_it.template emplace<N - 1>(ranges::end
                          (std::get<N - 1>(*m_views)));
          prev<N - 1>();
        } else {
          --std::get<N>(m_it);
        }
      }

      template<size_t N>
      constexpr void advance_fwd(difference_type offset, difference_type steps) {
        using Dt = iter_difference_t<variant_alternative_t<N, base_iter>>;
        if constexpr (N == sizeof...(Vs) - 1) {
          std::get<N>(m_it) += static_cast<Dt>(steps);
        } else {
          auto n_size = ranges::distance(std::get<N>(*m_views));
          if (offset + steps < n_size) {
            std::get<N>(m_it) += static_cast<Dt>(steps);
          } else {
            m_it.template emplace<N + 1>(ranges::begin(std::get<N + 1>(*m_views)));
            advance_fwd<N + 1>(0, offset + steps - n_size);
          }
        }
      }

      template<size_t N>
      constexpr void advance_bwd(difference_type offset, difference_type steps) {
        using Dt = iter_difference_t<variant_alternative_t<N, base_iter>>;
        if constexpr (N == 0) {
          std::get<N>(m_it) -= static_cast<Dt>(steps);
        } else if (offset >= steps) {
          std::get<N>(m_it) -= static_cast<Dt>(steps);
        } else {
          auto prev_size = ranges::distance(std::get<N - 1>(*m_views));
          m_it.template emplace<N - 1>(ranges::end(std::get<N - 1>(*m_views)));
          advance_bwd<N - 1>(prev_size, steps - offset);
        }
      }

      // Invoke the function object fun, which has a call operator with a size_t
      // template parameter (corresponding to an index into the pack of views),
      // using the runtime value of index as the template argument.
      template<typename Function>
      static constexpr auto invoke_with_runtime_index(Function&& fun, size_t index) {
        using ret_type = decltype(fun.template operator()<0>());
        using function_type = ret_type (*)(Function &&fun);
        static constexpr auto vtable = []<size_t ... Is>(index_sequence<Is ...>) {
          return array<function_type, sizeof...(Is)>{
            [](Function &&fun) {
              return fun.template operator()<Is>();
            } ...
          };
        }(index_sequence_for<Vs...>());
        return vtable[index](std::forward<Function>(fun));
      }

      template<typename... Args>
      explicit constexpr iterator(detail::maybe_const_t<IsConst, tuple<Vs...>>* views, Args&&... args)
        requires constructible_from<base_iter, Args&&...>
        : m_views(views), m_it(std::forward<Args>(args)...)
      { }

    public:
      iterator() = default;

      constexpr
      iterator(iterator<!IsConst> it)
        requires IsConst && (convertible_to<iterator_t<Vs>, iterator_t<const Vs>> && ...)
        : m_views(it.m_views)
        , m_it(invoke_with_runtime_index([this, &it]<size_t Idx>() {
            return base_iter(in_place_index<Idx>, std::get<Idx>(std::move(it.m_it)));
          }, it.m_it.index()))
      { }

      constexpr decltype(auto) operator*() const {
        assert(!m_it.valueless_by_exception());
        using reference = detail::concat_reference_t<detail::maybe_const_t<IsConst, Vs>...>;
        return std::visit([](auto&& it) -> reference { return *it; }, m_it);
      }

      constexpr iterator& operator++() {
        invoke_with_runtime_index([this]<size_t Idx>() {
          ++std::get<Idx>(m_it);
          satisfy<Idx>();
        }, m_it.index());
        return *this;
      }

      constexpr void operator++(int) { ++*this; }

      constexpr iterator operator++(int)
        requires detail::all_forward<IsConst, Vs...>
      {
        auto copy = *this;
        ++*this;
        return copy;
      }

      constexpr iterator& operator--()
        requires detail::concat_is_bidirectional<IsConst, Vs...>
      {
        assert(!m_it.valueless_by_exception());
        invoke_with_runtime_index([this]<size_t Idx>() {
          prev<Idx>();
        }, m_it.index());
        return *this;
      }

      constexpr iterator operator--(int)
        requires detail::concat_is_bidirectional<IsConst, Vs...>
      {
        auto copy = *this;
        --*this;
        return copy;
      }

      constexpr iterator& operator+=(difference_type n)
        requires detail::concat_is_random_access<IsConst, Vs...>
      {
        assert(!m_it.valueless_by_exception());
        invoke_with_runtime_index([this, n]<size_t Idx>() {
          auto begin = ranges::begin(std::get<Idx>(*m_views));
          if (n > 0)
            advance_fwd<Idx>(std::get<Idx>(m_it) - begin, n);
          else if (n < 0)
            advance_bwd<Idx>(std::get<Idx>(m_it) - begin, -n);
        }, m_it.index());
        return *this;
      }

      constexpr iterator& operator-=(difference_type n)
        requires detail::concat_is_random_access<IsConst, Vs...>
      {
        *this += -n;
        return *this;
      }

      constexpr decltype(auto) operator[](difference_type n) const
        requires detail::concat_is_random_access<IsConst, Vs...>
      {
        return *((*this) + n);
      }

      friend constexpr bool operator==(const iterator& x, const iterator& y)
        requires (equality_comparable<iterator_t<detail::maybe_const_t<IsConst, Vs>>> && ...)
      {
        assert(!x.m_it.valueless_by_exception());
        assert(!y.m_it.valueless_by_exception());
        return x.m_it == y.m_it;
      }

      friend constexpr bool operator==(const iterator& it, default_sentinel_t)
      {
        assert(!it.m_it.valueless_by_exception());
        constexpr auto lastIdx = sizeof...(Vs) - 1;
        return (it.m_it.index() == lastIdx
            && (std::get<lastIdx>(it.m_it)
            == ranges::end(std::get<lastIdx>(*it.m_views))));
      }

      friend constexpr bool operator<(const iterator& x, const iterator& y)
        requires detail::all_random_access<IsConst, Vs...>
      {
        return x.m_it < y.m_it;
      }

      friend constexpr bool operator>(const iterator& x, const iterator& y)
        requires detail::all_random_access<IsConst, Vs...>
      {
        return x.m_it > y.m_it;
      }

      friend constexpr bool operator<=(const iterator& x, const iterator& y)
        requires detail::all_random_access<IsConst, Vs...>
      {
        return x.m_it <= y.m_it;
      }

      friend constexpr bool operator>=(const iterator& x, const iterator& y)
        requires detail::all_random_access<IsConst, Vs...>
      {
        return x.m_it >= y.m_it;
      }

      friend constexpr auto operator<=>(const iterator& x, const iterator& y)
        requires detail::all_random_access<IsConst, Vs...>
        && (three_way_comparable<iterator_t<detail::maybe_const_t<IsConst, Vs>>> && ...)
      {
        return x.m_it <=> y.m_it;
      }

      friend constexpr iterator operator+(const iterator& it, difference_type n)
        requires detail::concat_is_random_access<IsConst, Vs...>
      {
        auto copy = it;
        copy += n;
        return copy;
      }

      friend constexpr iterator operator+(difference_type n, const iterator& it)
        requires detail::concat_is_random_access<IsConst, Vs...>
      {
        return it + n;
      }

      friend constexpr iterator operator-(const iterator& it, difference_type n)
        requires detail::concat_is_random_access<IsConst, Vs...>
      {
        auto copy = it;
        copy -= n;
        return copy;
      }

      friend constexpr difference_type operator-(const iterator& x, const iterator& y)
        requires detail::concat_is_random_access<IsConst, Vs...>
      {
        return invoke_with_runtime_index([&]<size_t Ix>() -> difference_type {
          return invoke_with_runtime_index([&]<size_t Iy>() -> difference_type {
            if constexpr (Ix > Iy) {
              auto dy = ranges::distance(std::get<Iy>(y.m_it),
                          ranges::end(std::get<Iy>(*y.m_views)));
              auto dx = ranges::distance(ranges::begin(std::get<Ix>(*x.m_views)),
                          std::get<Ix>(x.m_it));
              difference_type s = 0;
              [&]<size_t Idx = Iy + 1>(this auto&& self) {
                if constexpr (Idx < Ix) {
                  s += ranges::size(std::get<Idx>(*x.m_views));
                  self.template operator()<Idx + 1>();
                }
              }();
              return dy + s + dx;
            } else if constexpr (Ix < Iy) {
              return -(y - x);
            } else {
              return std::get<Ix>(x.m_it) - std::get<Iy>(y.m_it);
            }
          }, y.m_it.index());
        }, x.m_it.index());
      }

      friend constexpr difference_type operator-(const iterator& x, default_sentinel_t)
        requires (sized_sentinel_for<sentinel_t<detail::maybe_const_t<IsConst, Vs>>,
          iterator_t<detail::maybe_const_t<IsConst, Vs>>> && ...)
          && detail::all_but_first_sized<detail::maybe_const_t<IsConst, Vs>...>::value
      {
        return invoke_with_runtime_index([&]<size_t Ix>() -> difference_type {
          auto dx = ranges::distance(std::get<Ix>(x.m_it),
                          ranges::end(std::get<Ix>(*x.m_views)));
          difference_type s = 0;
          [&]<size_t Idx = Ix + 1>(this auto&& self) {
            if constexpr (Idx < sizeof...(Vs)) {
              s += ranges::size(std::get<Idx>(*x.m_views));
              self.template operator()<Idx + 1>();
            }
          }();
          return -(dx + s);
        }, x.m_it.index());
      }

      friend constexpr difference_type operator-(default_sentinel_t, const iterator& x)
        requires (sized_sentinel_for<sentinel_t<detail::maybe_const_t<IsConst, Vs>>,
                    iterator_t<detail::maybe_const_t<IsConst, Vs>>> && ...)
          && detail::all_but_first_sized<detail::maybe_const_t<IsConst, Vs>...>::value
      {
        return -(x - default_sentinel);
      }

      friend constexpr decltype(auto) iter_move(const iterator& it) {
        using Res = detail::concat_rvalue_reference_t<detail::maybe_const_t<IsConst, Vs>...>;
        return std::visit([](const auto& i) -> Res {
          return ranges::iter_move(i);
        }, it.m_it);
      }

      friend constexpr void iter_swap(const iterator& x, const iterator& y)
        requires swappable_with<iter_reference_t<iterator>, iter_reference_t<iterator>>
          && (... && indirectly_swappable<iterator_t<detail::maybe_const_t<IsConst, Vs>>>)
      {
        std::visit([&]<typename T, typename U>(const T& it1, const U& it2) {
          if constexpr (is_same_v<T, U>)
            ranges::iter_swap(it1, it2);
          else
            ranges::swap(*it1, *it2);
        }, x.m_it, y.m_it);
      }
    };

  public:
    constexpr concat_view() = default;

    constexpr explicit concat_view(Vs... views)
      : m_views(std::move(views)...)
    { }

    constexpr iterator<false>
    begin() requires (!(detail::simple_view<Vs> && ...))
    {
      iterator<false> it(&m_views, in_place_index<0>, ranges::begin(std::get<0>(m_views)));
      it.template satisfy<0>();
      return it;
    }

    constexpr iterator<true> begin() const
      requires (range<const Vs> && ...) && detail::concatable<const Vs...>
    {
      iterator<true> it(&m_views, in_place_index<0>, ranges::begin(std::get<0>(m_views)));
      it.template satisfy<0>();
      return it;
    }

    constexpr auto end()
      requires (!(detail::simple_view<Vs> && ...))
    {
      constexpr auto n = sizeof...(Vs);
      if constexpr (detail::all_forward<false, Vs...>
          && common_range<std::tuple_element_t<n - 1, std::tuple<Vs...>>>)
        return iterator<false>(&m_views, in_place_index<n - 1>, ranges::end(std::get<n - 1>(m_views)));
      else
        return default_sentinel;
    }

    constexpr auto end() const
      requires (range<const Vs> && ...) && detail::concatable<const Vs...>
    {
      constexpr auto n = sizeof...(Vs);
      if constexpr (detail::all_forward<true, Vs...>
          && common_range<std::tuple_element_t<n - 1, std::tuple<const Vs...>>>)
        return iterator<true>(&m_views, in_place_index<n - 1>, ranges::end(std::get<n - 1>(m_views)));
      else
        return default_sentinel;
    }

    constexpr auto size()
      requires (sized_range<Vs>&&...)
    {
      return [this]<size_t ... Is>(index_sequence<Is...>) {
        return (ranges::size(get<Is>(m_views)) + ...);
      }(index_sequence_for<Vs...>());
    }

    constexpr auto size() const
      requires (sized_range<const Vs>&&...)
    {
      return [this]<size_t ... Is>(index_sequence<Is...>) {
        return (ranges::size(get<Is>(m_views)) + ...);
      }(index_sequence_for<Vs...>());
    }
  };

  template<typename... Rs> concat_view(Rs&&...) -> concat_view<views::all_t<Rs>...>;

  namespace views {
    namespace detail {
      template<typename... Ts>
      concept can_concat_view = requires { concat_view(std::declval<Ts>()...); };
    }

    struct concat_fun {
      template<typename... Ts>
        requires detail::can_concat_view<Ts...>
      constexpr auto operator() [[nodiscard]] (Ts&&... ts) const {
        return concat_view(std::forward<Ts>(ts)...);
      }

      template<input_range Range>
      constexpr auto operator() [[nodiscard]] (Range&& t) const {
        return views::all(std::forward<Range>(t));
      }
    };

    inline constexpr concat_fun concat;
  }
  
}

#endif