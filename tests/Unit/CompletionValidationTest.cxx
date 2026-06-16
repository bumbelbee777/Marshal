#include "Heat/CompletionValidation.hxx"
#include "Heat/LogPrimeOperator.hxx"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    using namespace Marshal::Heat;

    const LogPrimeOperator op2 = LogPrimeOperator::from_prime(2);
    const LogPrimeOperator op3 = LogPrimeOperator::from_prime(3);
    assert(std::fabs(op2.eigenvalue(1) - op3.eigenvalue(1)) > 0.01L);

    struct Mode {
        int p;
        int k;
        Real omega;
    };
    std::vector<Mode> modes;
    for (int p : {2, 3, 5, 7}) {
        const LogPrimeOperator op = LogPrimeOperator::from_prime(p);
        for (int k = 1; k <= 4; ++k) modes.push_back({p, k, op.eigenvalue(k)});
    }
    std::sort(modes.begin(), modes.end(),
              [](const Mode& a, const Mode& b) { return a.omega < b.omega; });

    int cluster_count = 0;
    size_t i = 0;
    while (i < modes.size()) {
        size_t j = i + 1;
        while (j < modes.size() && modes[j].omega - modes[i].omega <= 1e-12L) ++j;
        if (j - i >= 2) ++cluster_count;
        i = j;
    }
    assert(cluster_count == 0);

    const Real ratio = std::log(2.0L) / std::log(3.0L);
    int best_k = 0;
    int best_l = 0;
    Real best_delta = 1e300L;
    for (int l = 1; l <= 1000; ++l) {
        const int k = static_cast<int>(std::lround(ratio * l));
        if (k < 1 || k > 1000) continue;
        const Real d = std::fabs(static_cast<Real>(k) / static_cast<Real>(l) - ratio);
        if (d < best_delta) {
            best_delta = d;
            best_k = k;
            best_l = l;
        }
    }
    assert(best_k >= 1 && best_l >= 1);
    assert(best_delta < 0.01L);

    std::cout << "CompletionValidationTest OK\n";
    return 0;
}
