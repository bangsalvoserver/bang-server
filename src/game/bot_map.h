#ifndef __BOT_MAP_H__
#define __BOT_MAP_H__

#include "player.h"

namespace banggame {

    struct bot_impl;

    class bot {
    private:
        std::unique_ptr<bot_impl> m_pimpl;

    public:
        bot(player *origin);
        ~bot();

        void request_play();
    };

    class bot_map {
    public:
        void add(player *p) {
            m_data.emplace(std::piecewise_construct, std::make_tuple(p), std::make_tuple(p));
        }

        bot *find(player *p) {
            auto it = m_data.find(p);
            if (it != m_data.end()) {
                return &it->second;
            }
            return nullptr;
        }

    private:
        std::map<player *, bot> m_data;
    };
}

#endif