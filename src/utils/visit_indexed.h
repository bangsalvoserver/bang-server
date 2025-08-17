#ifndef __VISIT_INDEXED_H__
#define __VISIT_INDEXED_H__

#include <concepts>
#include <functional>
#include <cassert>

namespace utils {

    namespace detail {
        template<typename T, size_t ... Dimensions>
        struct multi_array;

        template<typename T>
        struct multi_array<T> {
            T m_value;

            constexpr const T &get() const {
                return m_value;
            }
        };

        template<typename T, size_t First, size_t ... Rest>
        struct multi_array<T, First, Rest...> {
            using element_type = multi_array<T, Rest ...>;
            static constexpr size_t size = First;

            std::array<element_type, size> m_array;

            template<std::convertible_to<size_t> ... Is>
            constexpr decltype(auto) get(size_t first_index, Is ... rest_indices) const {
                assert(first_index < size);
                return m_array[first_index].get(rest_indices ...);
            }
        };
        
        template<typename ArrayType, typename ISeq>
        struct gen_vtable_impl;

        template<typename T, size_t ... Dimensions, size_t ... Indices>
        struct gen_vtable_impl<multi_array<T, Dimensions ...>, std::index_sequence<Indices ...>> {
            using array_type = multi_array<T, Dimensions ...>;

            static constexpr array_type apply() {
                return []<size_t ... Is>(std::index_sequence<Is ...>) {
                    return array_type{{
                        gen_vtable_impl<typename array_type::element_type, std::index_sequence<Indices ..., Is>>::apply() ...
                    }};
                }(std::make_index_sequence<array_type::size>());
            }
        };

        template<typename RetType, typename Visitor, size_t ... Indices>
        struct gen_vtable_impl<multi_array<RetType (*)(Visitor&&)>, std::index_sequence<Indices ...>> {
            using array_type = multi_array<RetType (*)(Visitor&&)>;

            static constexpr RetType invoke_visitor(Visitor &&visitor) {
                return std::invoke_r<RetType>(
                    std::forward<Visitor>(visitor),
                    std::integral_constant<size_t, Indices>{} ...
                );
            }

            static constexpr auto apply() {
                return array_type{invoke_visitor};
            }
        };

        template<typename RetType, typename Visitor, size_t ... Dimensions>
        struct gen_vtable {
            using array_type = multi_array<RetType (*)(Visitor&&), Dimensions ...>;
            static constexpr array_type value = gen_vtable_impl<array_type, std::index_sequence<>>::apply();
        };
    }

    template<size_t ... Dimensions>
    struct visit_indexed_t {
        template<typename Function, std::convertible_to<size_t> ... Is>
        auto operator()(Function &&fun, Is ... indices) const {
            using ret_type = decltype(std::invoke(fun, std::integral_constant<size_t, Dimensions * 0>{} ...));
            static constexpr auto vtable = detail::gen_vtable<ret_type, Function, Dimensions ...>::value;
            return std::invoke(vtable.get(indices ...), std::forward<Function>(fun));
        }
    };

    template<size_t ... Dimensions>
    inline constexpr visit_indexed_t<Dimensions ...> visit_indexed;

    template<typename RetType, size_t ... Dimensions>
    struct visit_indexed_r_t {
        template<typename Function, std::convertible_to<size_t> ... Is>
        auto operator()(Function &&fun, Is ... indices) const {
            static constexpr auto vtable = detail::gen_vtable<RetType, Function, Dimensions ...>::value;
            return std::invoke(vtable.get(indices ...), std::forward<Function>(fun));
        }
    };

    template<typename RetType, size_t ... Dimensions>
    inline constexpr visit_indexed_r_t<RetType, Dimensions ...> visit_indexed_r;
}

#endif