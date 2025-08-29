#ifndef __TSQUEUE_H__
#define __TSQUEUE_H__

#include <deque>
#include <optional>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace utils {

    template<typename T>
    class tsqueue {
    private:
        std::deque<T> m_queue;
        mutable std::mutex m_mutex;
        std::condition_variable m_cv;

    public:
        void push(const T &value) {
            std::scoped_lock lock{m_mutex};
            m_queue.push_back(value);
            m_cv.notify_one();
        }

        void push(T &&value) {
            std::scoped_lock lock{m_mutex};
            m_queue.push_back(std::move(value));
            m_cv.notify_one();
        }

        template<typename ... Ts>
        void emplace(Ts && ... args) {
            std::scoped_lock lock{m_mutex};
            m_queue.emplace_back(std::forward<Ts>(args) ...);
            m_cv.notify_one();
        }

        std::optional<T> pop() {
            std::scoped_lock lock{m_mutex};
            std::optional<T> result;
            if (!m_queue.empty()) {
                result = std::move(m_queue.front());
                m_queue.pop_front();
            }
            return result;
        }

        template<typename Rep, typename Period>
        std::optional<T> wait(std::chrono::duration<Rep, Period> rel_time) {
            std::scoped_lock lock{m_mutex};
            std::optional<T> result;
            if (m_cv.wait(lock, [&]{ return !m_queue.empty(); })) {
                result = std::move(m_queue.front());
                m_queue.pop_front();
            }
            return result;
        }
    };
    
}

#endif