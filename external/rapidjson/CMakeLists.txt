include(FetchContent)

FetchContent_Declare(
    rapidjson_content
    GIT_REPOSITORY https://github.com/Tencent/rapidjson
    GIT_TAG 24b5e7a8b27f42fa16b96fc70aade9106cf7102f
)
FetchContent_Populate(rapidjson_content)

add_library(RapidJSON INTERFACE)

target_include_directories(RapidJSON INTERFACE "${rapidjson_content_SOURCE_DIR}/include")