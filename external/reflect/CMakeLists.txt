include(FetchContent)

FetchContent_Declare(
    reflect_content
    GIT_REPOSITORY https://github.com/qlibs/reflect
    GIT_TAG v1.2.3
)
FetchContent_MakeAvailable(reflect_content)
add_library(reflect INTERFACE)

target_include_directories(reflect INTERFACE "${reflect_content_SOURCE_DIR}")
target_compile_definitions(reflect INTERFACE NTEST)