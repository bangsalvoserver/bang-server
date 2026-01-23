#ifndef __UTILS_INT_SET_H__
#define __UTILS_INT_SET_H__

#include <vector>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <bit>

namespace utils {

    class int_set {
    private:
        using value_type = uint64_t;

        struct set_chunk {
            value_type offset;
            value_type value;

            static set_chunk for_elem(value_type elem) {
                static constexpr value_type mask = std::numeric_limits<value_type>::digits - 1;
                return {
                    .offset = elem & ~mask,
                    .value = static_cast<value_type>(1) << (elem & mask)
                };
            }
        };

        std::vector<set_chunk> m_chunks;

        auto find_chunk(this auto &&self, value_type offset) {
            return std::ranges::lower_bound(self.m_chunks, offset, {}, &set_chunk::offset);
        }

    public:
        void add(value_type elem) {
            auto chunk = set_chunk::for_elem(elem);
            auto it = find_chunk(chunk.offset);
            if (it != m_chunks.end() && it->offset == chunk.offset) {
                it->value |= chunk.value;
            } else {
                m_chunks.insert(it, chunk);
            }
        }

        void remove(value_type elem) {
            auto chunk = set_chunk::for_elem(elem);
            auto it = find_chunk(chunk.offset);
            if (it != m_chunks.end() && it->offset == chunk.offset) {
                it->value &= ~chunk.value;
                if (it->value == 0) {
                    m_chunks.erase(it);
                }
            }
        }

        bool contains(value_type elem) const {
            auto chunk = set_chunk::for_elem(elem);
            auto it = find_chunk(chunk.offset);
            return it != m_chunks.end() && it->offset == chunk.offset
                && it->value & chunk.value;
        }

        size_t size() const {
            size_t result = 0;
            for (const set_chunk &chunk : m_chunks) {
                result += std::popcount(chunk.value);
            }
            return result;
        }

        bool empty() const {
            return m_chunks.empty();
        }
    };

}

#endif