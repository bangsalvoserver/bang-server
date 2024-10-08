#ifndef __TSQUEUE_H__
#define __TSQUEUE_H__

#include <deque>
#include <optional>
#include <thread>
#include <mutex>

namespace utils {

    template<typename T>
    class tsqueue {
    private:
        std::deque<T> m_queue;
        std::mutex m_mutex;

    public:
        void push(const T &value) {
            std::scoped_lock lock{m_mutex};
            m_queue.push_back(value);
        }

        void push(T &&value) {
            std::scoped_lock lock{m_mutex};
            m_queue.push_back(std::move(value));
        }

        template<typename ... Ts>
        void emplace(Ts && ... args) {
            std::scoped_lock lock{m_mutex};
            m_queue.emplace_back(std::forward<Ts>(args) ...);
        }

        std::optional<T> pop() {
            std::scoped_lock lock{m_mutex};
            if (m_queue.empty()) {
                return std::nullopt;
            }
            std::optional<T> result{std::move(m_queue.front())};
            m_queue.pop_front();
            return result;
        }
    };
    
}

#endif