#include <chrono>
#include <cstdio>
#include <vector>
#include "Marshal/Compat.hxx"
#include "Marshal/Kernel/FusedHotPaths.hxx"
#include "Marshal/Kernel/ScaleHotPaths.hxx"
#include "Marshal/Kernel/SimdMicrokernels.hxx"

namespace {

double bench_secs(auto&& fn, int reps) {
    auto t0 = std::chrono::steady_clock::now();
    double sink = 0;
    for (int r = 0; r < reps; ++r) sink += fn();
    auto t1 = std::chrono::steady_clock::now();
    (void)sink;
    return std::chrono::duration<double>(t1 - t0).count();
}

}  // namespace

int main() {
    constexpr size_t n = 1 << 20;
    std::vector<double> gamma(n);
    for (size_t i = 0; i < n; ++i) gamma[i] = 14.0 + static_cast<double>(i) * 0.001;
    const double t = 0.01;

#if defined(MARSHAL_HAVE_AVX2)
    const double zero_chunk =
        Marshal::Kernel::FusedZeroGaussianSum4(gamma.data(), n, t);
    const double zero_scale =
        Marshal::Kernel::FusedZeroGaussianSumScale(gamma.data(), n, t);
    const double zero_sec = bench_secs(
        [&] { return Marshal::Kernel::FusedZeroGaussianSumScale(gamma.data(), n, t); }, 8);
    std::printf("bench-micro zero: n=%zu chunk=%.6e scale=%.6e delta=%.3e elems/sec=%.3e\n",
                n, zero_chunk, zero_scale, zero_scale - zero_chunk, n / zero_sec);

    constexpr int k_cap = 64;
    const double log_p = std::log(7919.0);
    const double prime_scalar = [&] {
        double s = 0;
        double ppow = std::exp(log_p * 0.5);
        for (int k = 1; k <= k_cap; ++k) {
            const double u = static_cast<double>(k) * log_p;
            s += (log_p / ppow) * std::exp(-t * u * u);
            ppow *= std::exp(log_p * 0.5);
        }
        return s / (2.0 * 3.141592653589793);
    }();
    const double prime_simd = Marshal::Kernel::PrimePowerExpSum(
        log_p, k_cap, -t, 0.0, 2.0 * 3.141592653589793, true);
    const double prime_sec = bench_secs(
        [&] {
            return Marshal::Kernel::PrimePowerExpSum(log_p, k_cap, -t, 0.0,
                                                     2.0 * 3.141592653589793, true);
        },
        200000);
    std::printf("bench-micro prime: simd=%.6e scalar=%.6e delta=%.3e calls/sec=%.3e\n",
                prime_simd, prime_scalar, prime_simd - prime_scalar, 200000.0 / prime_sec);
#else
    double sum = 0;
    for (size_t i = 0; i < n; ++i) sum += 2.0 * std::exp(-t * gamma[i] * gamma[i]);
    std::printf("bench-micro scalar: n=%zu sum=%.6e\n", n, sum);
#endif
    return 0;
}
