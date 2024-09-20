# websocketpp interface library

add_library(websocketpp INTERFACE)

# threads

find_package(Threads REQUIRED)
target_link_libraries(websocketpp INTERFACE Threads::Threads)

# asio

target_compile_definitions(websocketpp INTERFACE ASIO_STANDALONE)

if(MINGW)
    target_link_libraries(websocketpp INTERFACE wsock32 ws2_32)
endif()

# openssl

find_package(OpenSSL)
if (NOT OPENSSL_FOUND)
    message(WARNING "Could not find OpenSSL")
    target_compile_definitions(websocketpp INTERFACE WSSERVER_NO_TLS)
else()
    target_link_libraries(websocketpp INTERFACE OpenSSL::SSL OpenSSL::Crypto)
endif()

# websocketpp

include(FetchContent)

FetchContent_Populate(websocketpp_content
    QUIET
    GIT_REPOSITORY http://github.com/zaphoyd/websocketpp
    GIT_TAG "b9aeec6eaf3d5610503439b4fae3581d9aff08e8"
)

target_include_directories(websocketpp INTERFACE "${websocketpp_content_SOURCE_DIR}")
target_compile_definitions(websocketpp INTERFACE _WEBSOCKETPP_CPP11_STL_)
target_compile_definitions(websocketpp INTERFACE _WEBSOCKETPP_CPP11_THREAD_)
target_compile_definitions(websocketpp INTERFACE _WEBSOCKETPP_NO_EXCEPTIONS_)