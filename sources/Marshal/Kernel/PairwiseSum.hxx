#pragma once
#include <algorithm>
#include <vector>
#include "../Numerics/Real.hxx"

namespace Marshal::Kernel {

inline Real PairwiseSum(const std::vector<Real>& v) {
    if (v.empty()) return Real{0};
    std::vector<Real> cur = v;
    while (cur.size() > 1) {
        std::vector<Real> nxt;
        nxt.reserve((cur.size() + 1) / 2);
        for (size_t i = 0; i < cur.size(); i += 2) {
            if (i + 1 < cur.size()) nxt.push_back(cur[i] + cur[i + 1]);
            else nxt.push_back(cur[i]);
        }
        cur.swap(nxt);
    }
    return cur[0];
}

}  // namespace Marshal::Kernel
