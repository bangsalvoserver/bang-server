#include "game_options.h"

#include "utils/static_map.h"

#include <meta>

namespace banggame {

    static constexpr auto game_options_fields = std::define_static_array(
        std::meta::nonstatic_data_members_of(^^game_options, std::meta::access_context::current())
    );

    std::string game_options::to_string(std::string_view sep) const {
        std::string result;

        bool first = true;
        template for(constexpr std::meta::info field : game_options_fields) {
            if (first) {
                first = false;
            } else {
                result += sep;
            }
            result += std::format("{} = {}", std::meta::identifier_of(field), this->[:field:]);
        }
        return result;
    }

    template<std::meta::info field, typename T>
    static T transform_game_option(const T &value) {
        template for (constexpr std::meta::info annotation : std::define_static_array(std::meta::annotations_of(field))) {
            constexpr auto type = std::meta::type_of(annotation);
            if constexpr (std::meta::has_template_arguments(type) && std::meta::template_of(type) == ^^transform) {
                return std::meta::extract<typename [:type:]>(annotation).transformer(value);
            }
        }

        return value;
    }

    template<std::meta::info field>
    static void set_value_fn(game_options &options, std::string_view str) {
        auto &field_value = options.[:field:];
        using field_type = std::remove_reference_t<decltype(field_value)>;

        if (std::optional<field_type> value = utils::parse_string<field_type>(str)) {
            field_value = transform_game_option<field>(*value);
        } else {
            throw game_option_error("INVALID_OPTION_VALUE");
        }
    }

    using set_value_fn_t = void (*)(game_options &options, std::string_view str);

    void game_options::set_option(std::string_view key, std::string_view value) {
        static constexpr auto set_option_map = []<size_t ... Is>(std::index_sequence<Is ...>) {
            return utils::make_static_map<std::string_view, set_value_fn_t>({
                {
                    std::meta::identifier_of(game_options_fields[Is]),
                    set_value_fn<game_options_fields[Is]>
                } ...
            });
        }(std::make_index_sequence<game_options_fields.size()>());
        
        auto it = set_option_map.find(key);
        if (it == set_option_map.end()) {
            throw game_option_error("INVALID_OPTION_NAME");
        }
        
        it->second(*this, value);
    }

    template<std::meta::info field>
    static void deserialize_field(game_options &result, const json::json &value) {
        try {
            auto &field_value = result.[:field:];
            using field_type = std::remove_reference_t<decltype(field_value)>;

            field_value = transform_game_option<field>(json::deserialize<field_type>(value));
        } catch (const std::exception &error) {
            // ignore errors.
            // game_options are stored in the clients' application storage and we don't want them kicked out if it's invalid.
        }
    }
    
    game_options game_options::deserialize_json(const json::json &value) {
        game_options result{};
        if (value.IsObject()) {
            template for (constexpr std::meta::info field : game_options_fields) {
                std::string_view member_name = std::meta::identifier_of(field);
                json::json key(rapidjson::StringRef(member_name.data(), member_name.size()));
                if (auto it = value.FindMember(key); it != value.MemberEnd()) {
                    deserialize_field<field>(result, it->value);
                }
            }
        }
        return result;
    }

}