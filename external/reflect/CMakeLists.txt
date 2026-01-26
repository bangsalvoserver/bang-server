include(FetchContent)

FetchContent_Declare(
    reflect_content
    GIT_REPOSITORY https://github.com/qlibs/reflect
    GIT_TAG 96b9548a6e114368a3ab22a674a0feab7da7cfd8
)
FetchContent_MakeAvailable(reflect_content)
add_library(reflect INTERFACE)

target_include_directories(reflect INTERFACE "${reflect_content_SOURCE_DIR}")
target_compile_definitions(reflect INTERFACE NTEST)