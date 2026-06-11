#include "PrimeSieve.hxx"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace Marshal::Heat {

namespace {

inline void set_composite(uint64_t* words, int offset) {
    words[static_cast<size_t>(offset >> 6)] |= (UINT64_C(1) << (offset & 63));
}

inline bool is_composite(const uint64_t* words, int offset) {
    return (words[static_cast<size_t>(offset >> 6)] >> (offset & 63)) & 1u;
}

void sieve_small(int sqrt_n, std::vector<int>& small) {
    std::vector<uint8_t> base(static_cast<size_t>(sqrt_n) + 1, 1);
    base[0] = base[1] = 0;
    for (int i = 2; i <= sqrt_n; ++i) {
        if (!base[static_cast<size_t>(i)]) {
            continue;
        }
        small.push_back(i);
        for (int j = i * i; j <= sqrt_n; j += i) {
            base[static_cast<size_t>(j)] = 0;
        }
    }
}

void collect_segment(int low, int high, const uint64_t* words, std::vector<int>& out) {
    for (int i = low; i <= high; ++i) {
        if (i < 2) {
            continue;
        }
        if (!is_composite(words, i - low)) {
            out.push_back(i);
        }
    }
}

}  // namespace

std::vector<int> SievePrimes(int max_n) {
    if (max_n < 2) {
        return {};
    }
    const int sqrt_n = static_cast<int>(std::sqrt(static_cast<double>(max_n)));
    std::vector<int> small;
    small.reserve(static_cast<size_t>(sqrt_n / 8));
    sieve_small(sqrt_n, small);

    std::vector<int> primes;
    primes.reserve(static_cast<size_t>(max_n / static_cast<int>(std::log(std::max(3, max_n)))) + 64);
    for (int p : small) {
        if (p <= max_n) {
            primes.push_back(p);
        }
    }

    constexpr int kSeg = 1 << 20;
    const int n_segments = (max_n - (sqrt_n + 1) + kSeg) / kSeg + 1;
    std::vector<std::vector<int>> buckets(static_cast<size_t>(n_segments));

#ifdef _OPENMP
    #pragma omp parallel
#endif
    {
        std::vector<uint64_t> words(static_cast<size_t>((kSeg + 63) / 64));
#ifdef _OPENMP
        #pragma omp for schedule(dynamic, 1)
#endif
        for (int seg = 0; seg < n_segments; ++seg) {
            const int low = sqrt_n + 1 + seg * kSeg;
            if (low > max_n) {
                continue;
            }
            const int high = std::min(low + kSeg - 1, max_n);
            const int span = high - low + 1;
            std::fill(words.begin(), words.begin() + static_cast<size_t>((span + 63) / 64), 0);

            for (int p : small) {
                long long start = (static_cast<long long>(low) + p - 1) / p;
                start *= p;
                if (start < static_cast<long long>(p) * p) {
                    start = static_cast<long long>(p) * p;
                }
                for (long long j = start; j <= high; j += p) {
                    set_composite(words.data(), static_cast<int>(j - low));
                }
            }

            std::vector<int> local;
            local.reserve(static_cast<size_t>((high - low) / 8) + 8);
            collect_segment(low, high, words.data(), local);
            buckets[static_cast<size_t>(seg)] = std::move(local);
        }
    }

    for (const auto& b : buckets) {
        primes.insert(primes.end(), b.begin(), b.end());
    }
    return primes;
}

}  // namespace Marshal::Heat
