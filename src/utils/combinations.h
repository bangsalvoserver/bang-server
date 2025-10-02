#ifndef __UTILS_COMBINATIONS_H__
#define __UTILS_COMBINATIONS_H__

#include <generator>
#include <vector>

namespace utils {

    template<typename T>
    std::generator<std::vector<T>> combinations(std::vector<T> elems, size_t n) {
        assert(n != 0);
        
        if (elems.size() < n) co_return;

        std::vector<size_t> idx(n);
        for (size_t i = 0; i < n; ++i) idx[i] = i;

        while (true) {
            std::vector<T> sel;
            sel.reserve(n);
            for (size_t i = 0; i < n; ++i) sel.emplace_back(elems[idx[i]]);
            co_yield sel;

            // advance lexicographically
            int i = static_cast<int>(n) - 1;
            for (; i >= 0; --i) {
                if (idx[i] + (n - i) < elems.size()) {
                    ++idx[i];
                    for (size_t j = i + 1; j < n; ++j) idx[j] = idx[j - 1] + 1;
                    break;
                }
            }
            if (i < 0) break;
        }
    }

}

#endif