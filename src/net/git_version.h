#ifndef __NET_GIT_VERSION_H__
#define __NET_GIT_VERSION_H__

#include <string_view>

namespace net {

#ifdef NDEBUG

    extern const std::string_view server_commit_hash;

    inline bool validate_commit_hash(std::string_view commit_hash) {
        return commit_hash == server_commit_hash;
    }

#else

    constexpr bool validate_commit_hash(std::string_view) {
        return true;
    }

#endif

}

#endif