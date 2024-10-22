include(FetchContent)

# uSockets

FetchContent_Declare(
    uSockets_content
    GIT_REPOSITORY https://github.com/uNetworking/uSockets
    GIT_TAG v0.8.8
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(uSockets_content)
file(GLOB_RECURSE SOURCES ${usockets_content_SOURCE_DIR}/src/*.c ${usockets_content_SOURCE_DIR}/src/*.cpp)
add_library(uSockets ${SOURCES})
target_include_directories(uSockets PUBLIC ${usockets_content_SOURCE_DIR}/src)

if (WIN32)
    target_link_libraries(uSockets PUBLIC wsock32 ws2_32)
endif()

# libuv

find_path(LIBUV_INCLUDE_DIR NAMES uv.h)
find_library(LIBUV_LIBRARIES NAMES uv libuv)

add_library(libuv INTERFACE)
target_include_directories(libuv INTERFACE ${LIBUV_INCLUDE_DIR})
target_link_libraries(libuv INTERFACE ${LIBUV_LIBRARIES})

target_link_libraries(uSockets PUBLIC libuv)
target_compile_definitions(uSockets PUBLIC LIBUS_USE_LIBUV)

# openssl

find_package(OpenSSL)
if (NOT OPENSSL_FOUND)
    message(WARNING "Could not find OpenSSL")
    target_compile_definitions(uSockets PUBLIC LIBUS_NO_SSL)
else()
    target_link_libraries(uSockets INTERFACE OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(uSockets PUBLIC LIBUS_USE_OPENSSL)
endif()

# zlib

find_package(ZLIB REQUIRED)

# uWebSockets

FetchContent_Declare(
    uWebSockets_content
    GIT_REPOSITORY https://github.com/uNetworking/uWebSockets
    GIT_TAG v20.68.0
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(uWebSockets_content)
add_library(uwebsockets INTERFACE)
target_include_directories(uwebsockets INTERFACE ${uwebsockets_content_SOURCE_DIR}/src/)
target_link_libraries(uwebsockets INTERFACE uSockets ZLIB::ZLIB)