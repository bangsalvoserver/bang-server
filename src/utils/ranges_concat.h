#ifndef __RANGES_CONCAT_H__
#define __RANGES_CONCAT_H__

#include <ranges>
#include <tuple>
#include <variant>
#include <cassert>

namespace std::ranges
{
  namespace __detail
  {
    template<typename _Range>
      concept my_simple_view = view<_Range> && range<const _Range>
	&& same_as<iterator_t<_Range>, iterator_t<const _Range>>
	&& same_as<sentinel_t<_Range>, sentinel_t<const _Range>>;
    
    template<bool _Const, typename _Tp>
        using my_maybe_const_t = conditional_t<_Const, const _Tp, _Tp>;

    template<bool _Const, typename... _Vs>
      concept my_all_random_access
	= (random_access_range<my_maybe_const_t<_Const, _Vs>> && ...);

    template<bool _Const, typename... _Vs>
      concept my_all_bidirectional
	= (bidirectional_range<my_maybe_const_t<_Const, _Vs>> && ...);

    template<bool _Const, typename... _Vs>
      concept my_all_forward
	= (forward_range<my_maybe_const_t<_Const, _Vs>> && ...);

    template<typename... _Rs>
      using __concat_reference_t = common_reference_t<range_reference_t<_Rs>...>;

    template<typename... _Rs>
      using __concat_value_t = common_type_t<range_value_t<_Rs>...>;

    template<typename... _Rs>
      using __concat_rvalue_reference_t
	= common_reference_t<range_rvalue_reference_t<_Rs>...>;

    template<typename _Ref, typename _RRef, typename _It>
      concept __concat_indirectly_readable_impl = requires(const _It __it) {
	{ *__it } -> convertible_to<_Ref>;
	{ ranges::iter_move(__it) } -> convertible_to<_RRef>;
      };

    template<typename... _Rs>
      concept __concat_indirectly_readable
	= common_reference_with<__concat_reference_t<_Rs...>&&, __concat_value_t<_Rs...>&>
	  && common_reference_with<__concat_reference_t<_Rs...>&&,
				   __concat_rvalue_reference_t<_Rs...>&&>
	  && common_reference_with<__concat_rvalue_reference_t<_Rs...>&&,
				   __concat_value_t<_Rs...> const&>
	  && (__concat_indirectly_readable_impl<__concat_reference_t<_Rs...>,
						__concat_rvalue_reference_t<_Rs...>,
						iterator_t<_Rs>>
	      && ...);

    template<typename... _Rs>
      concept __concatable = requires {
	typename __concat_reference_t<_Rs...>;
	typename __concat_value_t<_Rs...>;
	typename __concat_rvalue_reference_t<_Rs...>;
      } && __concat_indirectly_readable<_Rs...>;

    template<bool _Const, typename _Range, typename... _Rs>
      struct __all_but_last_common
      {
	static inline constexpr bool value
	  = requires { requires (common_range<my_maybe_const_t<_Const, _Range>>
				 && __all_but_last_common<_Const, _Rs...>::value); };
      };

    template<bool _Const, typename _Range>
      struct __all_but_last_common<_Const, _Range>
      { static inline constexpr bool value = true; };

    template<bool _Const, typename... _Rs>
      concept __concat_is_random_access = my_all_random_access<_Const, _Rs...>
	&& __all_but_last_common<_Const, _Rs...>::value;

    template<bool _Const, typename... _Rs>
      concept __concat_is_bidirectional = my_all_bidirectional<_Const, _Rs...>
	&& __all_but_last_common<_Const, _Rs...>::value;

    template<typename _Range, typename... _Rs>
      struct __all_but_first_sized
      { static inline constexpr bool value = (sized_range<_Rs> && ...); };
  } // namespace __detail

  template<bool _Const, input_range... _Vs>
    requires (view<_Vs> && ...) && (sizeof...(_Vs) > 0) && __detail::__concatable<_Vs...>
  class __concat_view_iterator;

  template<input_range... _Vs>
    requires (view<_Vs> && ...) && (sizeof...(_Vs) > 0) && __detail::__concatable<_Vs...>
  class concat_view : public view_interface<concat_view<_Vs...>>
  {
    tuple<_Vs...> _M_views;

    template<bool _Const> using _Iterator = __concat_view_iterator<_Const, _Vs...>;

  public:
    constexpr concat_view() = default;

    constexpr explicit
    concat_view(_Vs... __views)
    : _M_views(std::move(__views)...)
    { }

    constexpr _Iterator<false>
    begin() requires (!(__detail::my_simple_view<_Vs> && ...))
    {
      _Iterator<false> __it(&_M_views, in_place_index<0>, ranges::begin(std::get<0>(_M_views)));
      __it.template _M_satisfy<0>();
      return __it;
    }

    constexpr _Iterator<true>
    begin() const requires (range<const _Vs> && ...) && __detail::__concatable<const _Vs...>
    {
      _Iterator<true> __it(&_M_views, in_place_index<0>, ranges::begin(std::get<0>(_M_views)));
      __it.template _M_satisfy<0>();
      return __it;
    }

    constexpr auto
    end() requires (!(__detail::my_simple_view<_Vs> && ...))
    {
      constexpr auto __n = sizeof...(_Vs);
      if constexpr (__detail::my_all_forward<false, _Vs...>
		    && common_range<std::tuple_element_t<__n - 1, std::tuple<_Vs...>>>)
	return _Iterator<false>(&_M_views, in_place_index<__n - 1>,
				ranges::end(std::get<__n - 1>(_M_views)));
      else
	return default_sentinel;
    }

    constexpr auto
    end() const requires (range<const _Vs> && ...) && __detail::__concatable<const _Vs...>
    {
      constexpr auto __n = sizeof...(_Vs);
      if constexpr (__detail::my_all_forward<true, _Vs...>
		    && common_range<std::tuple_element_t<__n - 1, std::tuple<const _Vs...>>>)
	return _Iterator<true>(&_M_views, in_place_index<__n - 1>,
			       ranges::end(std::get<__n - 1>(_M_views)));
      else
	return default_sentinel;
    }

    constexpr auto
    size() requires (sized_range<_Vs>&&...)
    {
      return [this]<size_t ... Is>(index_sequence<Is...>) {
        return (ranges::size(get<Is>(_M_views)) + ...);
      }(index_sequence_for<_Vs...>());
    }

    constexpr auto
    size() const requires (sized_range<const _Vs>&&...)
    {
      return [this]<size_t ... Is>(index_sequence<Is...>) {
        return (ranges::size(get<Is>(_M_views)) + ...);
      }(index_sequence_for<_Vs...>());
    }
  };

  template<typename... _Rs>
    concat_view(_Rs&&...) -> concat_view<views::all_t<_Rs>...>;

  namespace __detail
  {
    template<bool _Const, typename... _Vs>
      struct __concat_view_iter_cat
      { };

    template<bool _Const, typename... _Vs>
      requires __detail::my_all_forward<_Const, _Vs...>
      struct __concat_view_iter_cat<_Const, _Vs...>
      {
	static auto
	_S_iter_cat()
	{
	  if constexpr (!is_reference_v<__concat_reference_t<my_maybe_const_t<_Const, _Vs>...>>)
	    return input_iterator_tag{};
	  else
	    return []<typename... _Cats>(_Cats... __cats) {
	      if constexpr ((derived_from<_Cats, random_access_iterator_tag> && ...)
			    && __concat_is_random_access<_Const, _Vs...>)
		return random_access_iterator_tag{};
	      else if constexpr ((derived_from<_Cats, bidirectional_iterator_tag> && ...)
				 && __concat_is_bidirectional<_Const, _Vs...>)
		return bidirectional_iterator_tag{};
	      else if constexpr ((derived_from<_Cats, forward_iterator_tag> && ...))
		return forward_iterator_tag{};
	      else
		return input_iterator_tag{};
	    }(typename iterator_traits<iterator_t<my_maybe_const_t<_Const, _Vs>>>
	      ::iterator_category{}...);
	}
      };
  }

  template<bool _Const, input_range... _Vs>
    requires (view<_Vs> && ...) && (sizeof...(_Vs) > 0) && __detail::__concatable<_Vs...>
  class __concat_view_iterator
  : public __detail::__concat_view_iter_cat<_Const, _Vs...>
  {
    static auto
    _S_iter_concept()
    {
      if constexpr (__detail::__concat_is_random_access<_Const, _Vs...>)
	return random_access_iterator_tag{};
      else if constexpr (__detail::__concat_is_bidirectional<_Const, _Vs...>)
	return bidirectional_iterator_tag{};
      else if constexpr (__detail::my_all_forward<_Const, _Vs...>)
	return forward_iterator_tag{};
      else
	return input_iterator_tag{};
    }

    friend concat_view<_Vs...>;
    friend __concat_view_iterator<!_Const, _Vs...>;

  public:
    // iterator_category defined in __concat_view_iter_cat
    using iterator_concept = decltype(_S_iter_concept());
    using value_type = __detail::__concat_value_t<__detail::my_maybe_const_t<_Const, _Vs>...>;
    using difference_type = common_type_t<range_difference_t<__detail::my_maybe_const_t<_Const, _Vs>>...>;

  private:
    using __base_iter = variant<iterator_t<__detail::my_maybe_const_t<_Const, _Vs>>...>;

    __detail::my_maybe_const_t<_Const, tuple<_Vs...>>* _M_parent_views = nullptr;
    __base_iter _M_it;

    template<size_t _Nm>
      constexpr void
      _M_satisfy()
      {
	if constexpr (_Nm < (sizeof...(_Vs) - 1))
	  {
	    if (std::get<_Nm>(_M_it) == ranges::end(std::get<_Nm>(*_M_parent_views)))
	      {
		_M_it.template emplace<_Nm + 1>(ranges::begin
						(std::get<_Nm + 1>(*_M_parent_views)));
		_M_satisfy<_Nm + 1>();
	      }
	  }
      }

    template<size_t _Nm>
      constexpr void
      _M_prev()
      {
	if constexpr (_Nm == 0)
	  --std::get<0>(_M_it);
	else
	  {
	    if (std::get<_Nm>(_M_it) == ranges::begin(std::get<_Nm>(*_M_parent_views)))
	      {
		_M_it.template emplace<_Nm - 1>(ranges::end
						(std::get<_Nm - 1>(*_M_parent_views)));
		_M_prev<_Nm - 1>();
	      }
	    else
	      --std::get<_Nm>(_M_it);
	  }
      }

    template<size_t _Nm>
      constexpr void
      _M_advance_fwd(difference_type __offset, difference_type __steps)
      {
	using _Dt = iter_difference_t<variant_alternative_t<_Nm, __base_iter>>;
	if constexpr (_Nm == sizeof...(_Vs) - 1)
	  std::get<_Nm>(_M_it) += static_cast<_Dt>(__steps);
	else
	  {
	    auto __n_size = ranges::distance(std::get<_Nm>(*_M_parent_views));
	    if (__offset + __steps < __n_size)
		std::get<_Nm>(_M_it) += static_cast<_Dt>(__steps);
	    else
	      {
		_M_it.template emplace<_Nm + 1>(ranges::begin
						(std::get<_Nm + 1>(*_M_parent_views)));
		_M_advance_fwd<_Nm + 1>(0, __offset + __steps - __n_size);
	      }
	  }
      }

    template<size_t _Nm>
      constexpr void
      _M_advance_bwd(difference_type __offset, difference_type __steps)
      {
	using _Dt = iter_difference_t<variant_alternative_t<_Nm, __base_iter>>;
	if constexpr (_Nm == 0)
	  std::get<_Nm>(_M_it) -= static_cast<_Dt>(__steps);
	else {
	    if (__offset >= __steps)
	      std::get<_Nm>(_M_it) -= static_cast<_Dt>(__steps);
	    else
	      {
		auto __prev_size = ranges::distance(std::get<_Nm - 1>(*_M_parent_views));
		_M_it.template emplace<_Nm - 1>(ranges::end
						(std::get<_Nm - 1>(*_M_parent_views)));
		_M_advance_bwd<_Nm - 1>(__prev_size, __steps - __offset);
	      }
	}
      }

    // Invoke the function object __f, which has a call operator with a size_t
    // template parameter (corresponding to an index into the pack of views),
    // using the runtime value of __index as the template argument.
    template<typename _Fp>
      static constexpr auto
      _S_invoke_with_runtime_index(_Fp&& __f, size_t __index)
      {
	return [&__f, __index]<size_t _Idx>(this auto&& __self) {
	  if (_Idx == __index)
	    return __f.template operator()<_Idx>();
	  if constexpr (_Idx + 1 < sizeof...(_Vs))
	    return __self.template operator()<_Idx + 1>();
	  std::unreachable();
	}.template operator()<0>();
      }

    template<typename _Fp>
      constexpr auto
      _M_invoke_with_runtime_index(_Fp&& __f)
      { return _S_invoke_with_runtime_index(std::forward<_Fp>(__f), _M_it.index()); }

    template<typename... _Args>
      explicit constexpr
      __concat_view_iterator(__detail::my_maybe_const_t<_Const, tuple<_Vs...>>* __parent_views, _Args&&... __args)
	requires constructible_from<__base_iter, _Args&&...>
      : _M_parent_views(__parent_views), _M_it(std::forward<_Args>(__args)...)
      { }

  public:
    __concat_view_iterator() = default;

    constexpr
    __concat_view_iterator(__concat_view_iterator<!_Const, _Vs...> __it)
      requires _Const && (convertible_to<iterator_t<_Vs>, iterator_t<const _Vs>> && ...)
    : _M_parent_views(__it._M_parent_views),
      _M_it(_S_invoke_with_runtime_index([this, &__it]<size_t _Idx>() {
	      return __base_iter(in_place_index<_Idx>,
				 std::get<_Idx>(std::move(__it._M_it)));
	    }, __it._M_it.index()))
    { }

    constexpr decltype(auto)
    operator*() const
    {
      assert(!_M_it.valueless_by_exception());
      using reference = __detail::__concat_reference_t<__detail::my_maybe_const_t<_Const, _Vs>...>;
      return std::visit([](auto&& __it) -> reference { return *__it; }, _M_it);
    }

    constexpr __concat_view_iterator&
    operator++()
    {
      _M_invoke_with_runtime_index([this]<size_t _Idx>() {
	++std::get<_Idx>(_M_it);
	_M_satisfy<_Idx>();
      });
      return *this;
    }

    constexpr void
    operator++(int)
    { ++*this; }

    constexpr __concat_view_iterator
    operator++(int)
      requires __detail::my_all_forward<_Const, _Vs...>
    {
      auto __tmp = *this;
      ++*this;
      return __tmp;
    }

    constexpr __concat_view_iterator&
    operator--()
      requires __detail::__concat_is_bidirectional<_Const, _Vs...>
    {
      assert(!_M_it.valueless_by_exception());
      _M_invoke_with_runtime_index([this]<size_t _Idx>() {
	_M_prev<_Idx>();
      });
      return *this;
    }

    constexpr __concat_view_iterator
    operator--(int)
      requires __detail::__concat_is_bidirectional<_Const, _Vs...>
    {
      auto __tmp = *this;
      --*this;
      return __tmp;
    }

    constexpr __concat_view_iterator&
    operator+=(difference_type __n)
      requires __detail::__concat_is_random_access<_Const, _Vs...>
    {
      assert(!_M_it.valueless_by_exception());
      _M_invoke_with_runtime_index([this, __n]<size_t _Idx>() {
	auto __begin = ranges::begin(std::get<_Idx>(*_M_parent_views));
	if (__n > 0)
	  _M_advance_fwd<_Idx>(std::get<_Idx>(_M_it) - __begin, __n);
	else if (__n < 0)
	  _M_advance_bwd<_Idx>(std::get<_Idx>(_M_it) - __begin, -__n);
      });
      return *this;
    }

    constexpr __concat_view_iterator&
    operator-=(difference_type __n)
      requires __detail::__concat_is_random_access<_Const, _Vs...>
    {
      *this += -__n;
      return *this;
    }

    constexpr decltype(auto)
    operator[](difference_type __n) const
      requires __detail::__concat_is_random_access<_Const, _Vs...>
    { return *((*this) + __n); }

    friend constexpr bool
    operator==(const __concat_view_iterator& __x, const __concat_view_iterator& __y)
      requires (equality_comparable<iterator_t<__detail::my_maybe_const_t<_Const, _Vs>>> && ...)
    {
      assert(!__x._M_it.valueless_by_exception());
      assert(!__y._M_it.valueless_by_exception());
      return __x._M_it == __y._M_it;
    }

    friend constexpr bool
    operator==(const __concat_view_iterator& __it, default_sentinel_t)
    {
      assert(!__it._M_it.valueless_by_exception());
      constexpr auto __last_idx = sizeof...(_Vs) - 1;
      return (__it._M_it.index() == __last_idx
	      && (std::get<__last_idx>(__it._M_it)
		  == ranges::end(std::get<__last_idx>(*__it._M_parent_views))));
    }

    friend constexpr bool
    operator<(const __concat_view_iterator& __x, const __concat_view_iterator& __y)
      requires __detail::my_all_random_access<_Const, _Vs...>
    { return __x._M_it < __y._M_it; }

    friend constexpr bool
    operator>(const __concat_view_iterator& __x, const __concat_view_iterator& __y)
      requires __detail::my_all_random_access<_Const, _Vs...>
    { return __x._M_it > __y._M_it; }

    friend constexpr bool
    operator<=(const __concat_view_iterator& __x, const __concat_view_iterator& __y)
      requires __detail::my_all_random_access<_Const, _Vs...>
    { return __x._M_it <= __y._M_it; }

    friend constexpr bool
    operator>=(const __concat_view_iterator& __x, const __concat_view_iterator& __y)
      requires __detail::my_all_random_access<_Const, _Vs...>
    { return __x._M_it >= __y._M_it; }

    friend constexpr auto
    operator<=>(const __concat_view_iterator& __x, const __concat_view_iterator& __y)
      requires __detail::my_all_random_access<_Const, _Vs...>
	&& (three_way_comparable<iterator_t<__detail::my_maybe_const_t<_Const, _Vs>>> && ...)
    { return __x._M_it <=> __y._M_it; }

    friend constexpr __concat_view_iterator
    operator+(const __concat_view_iterator& __it, difference_type __n)
      requires __detail::__concat_is_random_access<_Const, _Vs...>
    { auto copy = __it; copy += __n; return copy; }

    friend constexpr __concat_view_iterator
    operator+(difference_type __n, const __concat_view_iterator& __it)
      requires __detail::__concat_is_random_access<_Const, _Vs...>
    { return __it + __n; }

    friend constexpr __concat_view_iterator
    operator-(const __concat_view_iterator& __it, difference_type __n)
      requires __detail::__concat_is_random_access<_Const, _Vs...>
    { auto copy = __it; copy -= __n; return copy; }

    friend constexpr difference_type
    operator-(const __concat_view_iterator& __x, const __concat_view_iterator& __y)
      requires __detail::__concat_is_random_access<_Const, _Vs...>
    {
      return _S_invoke_with_runtime_index([&]<size_t _Ix>() -> difference_type {
        return _S_invoke_with_runtime_index([&]<size_t _Iy>() -> difference_type {
	  if constexpr (_Ix > _Iy)
	    {
	      auto __dy = ranges::distance(std::get<_Iy>(__y._M_it),
					   ranges::end(std::get<_Iy>(*__y._M_parent_views)));
	      auto __dx = ranges::distance(ranges::begin(std::get<_Ix>(*__x._M_parent_views)),
					   std::get<_Ix>(__x._M_it));
	      difference_type __s = 0;
	      [&]<size_t _Idx = _Iy + 1>(this auto&& __self) {
	        if constexpr (_Idx < _Ix)
		  {
		    __s += ranges::size(std::get<_Idx>(*__x._M_parent_views));
		    __self.template operator()<_Idx + 1>();
		  }
	      }();
	      return __dy + __s + __dx;
	    }
	  else if constexpr (_Ix < _Iy)
	    return -(__y - __x);
	  else
	    return std::get<_Ix>(__x._M_it) - std::get<_Iy>(__y._M_it);
	}, __y._M_it.index());
      }, __x._M_it.index());
    }

    friend constexpr difference_type
    operator-(const __concat_view_iterator& __x, default_sentinel_t)
      requires (sized_sentinel_for<sentinel_t<__detail::my_maybe_const_t<_Const, _Vs>>,
				   iterator_t<__detail::my_maybe_const_t<_Const, _Vs>>> && ...)
	&& __detail::__all_but_first_sized<__detail::my_maybe_const_t<_Const, _Vs>...>::value
    {
      return _S_invoke_with_runtime_index([&]<size_t _Ix>() -> difference_type {
	auto __dx = ranges::distance(std::get<_Ix>(__x._M_it),
				     ranges::end(std::get<_Ix>(*__x._M_parent_views)));
	difference_type __s = 0;
	[&]<size_t _Idx = _Ix + 1>(this auto&& __self) {
	  if constexpr (_Idx < sizeof...(_Vs))
	    {
	      __s += ranges::size(std::get<_Idx>(*__x._M_parent_views));
	      __self.template operator()<_Idx + 1>();
	    }
	}();
	return -(__dx + __s);
      }, __x._M_it.index());
    }

    friend constexpr difference_type
    operator-(default_sentinel_t, const __concat_view_iterator& __x)
      requires (sized_sentinel_for<sentinel_t<__detail::my_maybe_const_t<_Const, _Vs>>,
				   iterator_t<__detail::my_maybe_const_t<_Const, _Vs>>> && ...)
	&& __detail::__all_but_first_sized<__detail::my_maybe_const_t<_Const, _Vs>...>::value
    { return -(__x - default_sentinel); }

    friend constexpr decltype(auto)
    iter_move(const __concat_view_iterator& __it)
    {
      using _Res = __detail::__concat_rvalue_reference_t<__detail::my_maybe_const_t<_Const, _Vs>...>;
      return std::visit([](const auto& __i) -> _Res {
	return ranges::iter_move(__i);
      }, __it._M_it);
    }

    friend constexpr void
    iter_swap(const __concat_view_iterator& __x, const __concat_view_iterator& __y)
      requires swappable_with<iter_reference_t<__concat_view_iterator>, iter_reference_t<__concat_view_iterator>>
	&& (... && indirectly_swappable<iterator_t<__detail::my_maybe_const_t<_Const, _Vs>>>)
    {
      std::visit([&]<typename _Tp, typename _Up>(const _Tp& __it1, const _Up& __it2) {
	if constexpr (is_same_v<_Tp, _Up>)
	  ranges::iter_swap(__it1, __it2);
	else
	  ranges::swap(*__it1, *__it2);
      }, __x._M_it, __y._M_it);
    }
  };

  namespace views
  {
    namespace __detail
    {
      template<typename... _Ts>
	concept __can_concat_view = requires { concat_view(std::declval<_Ts>()...); };
    }

    struct _Concat
    {
      template<typename... _Ts>
	requires __detail::__can_concat_view<_Ts...>
      constexpr auto
      operator() [[nodiscard]] (_Ts&&... __ts) const
      { return concat_view(std::forward<_Ts>(__ts)...); }

      template<input_range _Range>
      constexpr auto
      operator() [[nodiscard]] (_Range&& __t) const
      { return views::all(std::forward<_Range>(__t)); }
    };

    inline constexpr _Concat concat;
  }
  
}

#endif