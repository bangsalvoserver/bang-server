#ifndef __VISIT_ENUM_H__
#define __VISIT_ENUM_H__

#include "enums.h"

namespace enums {

    template<enumeral auto E> struct enum_tag_t {};

    template<enumeral auto E> inline constexpr enum_tag_t<E> enum_tag;
    
    template<typename RetType, typename Visitor, enumeral E>
    RetType visit_enum(Visitor &&visitor, E value) {
        static constexpr auto vtable = []<size_t ... Is>(std::index_sequence<Is ...>) {
            return std::array<RetType (*)(Visitor&&), sizeof...(Is)> {
                [](Visitor &&visitor) -> RetType {
                    return std::invoke(std::forward<Visitor>(visitor), enum_tag<enums::enum_values<E>()[Is]>);
                } ...
            };
        }(std::make_index_sequence<enums::enum_values<E>().size()>());

        return vtable[indexof(value)](std::forward<Visitor>(visitor));
    }

    template<typename Visitor, enumeral E>
    decltype(auto) visit_enum(Visitor &&visitor, E value) {
        using return_type = std::invoke_result_t<Visitor, enum_tag_t<enums::enum_values<E>()[0]>>;
        return visit_enum<return_type>(std::forward<Visitor>(visitor), value);
    }
}

#endif