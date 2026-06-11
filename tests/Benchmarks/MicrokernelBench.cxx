#include <chrono>
#include <cstdio>
#include "Marshal/Compat.hxx"
#include "Marshal/Kernel/FusedHotPaths.hxx"

int main() {
    constexpr size_t n = 1 << 20;
    std::vector<double> gamma(n);
    for (size_t i = 0; i < n; ++i) gamma[i] = 14.0 + static_cast<double>(i) * 0.001;
    const double t = 0.01;

    auto t0 = std::chrono::steady_clock::now();
    double sum = 0;
#if defined(MARSHAL_HAVE_AVX2)
    sum = Marshal::Kernel::FusedZeroGaussianSum4(gamma.data(), n, t);
#else
    for (size_t i = 0; i < n; ++i) sum += 2.0 * std::exp(-t * gamma[i] * gamma[i]);
#endif
    auto t1 = std::chrono::steady_clock::now();
    const double sec = std::chrono::duration<double>(t1 - t0).count();
    std::printf("bench-micro: n=%zu sum=%.6e elems/sec=%.3e\n", n, sum, n / sec);
    return 0;
}
