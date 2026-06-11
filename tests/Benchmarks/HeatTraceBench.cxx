#include <chrono>
#include <cstdio>
#include <vector>
#include "Marshal/Compat.hxx"
#include "Marshal/Kernel/FusedHotPaths.hxx"

int main() {
    const Real t = 0.01L;
    const Real inv_log2 = 1.0L / std::log(2.0L);
    constexpr int nmax = 5000;
    constexpr int n_primes = 4096;

    std::vector<double> inv_log_p(n_primes);
    for (int i = 0; i < n_primes; ++i)
        inv_log_p[static_cast<size_t>(i)] = 1.0 / std::log(static_cast<double>(i + 2));

    auto t0 = std::chrono::steady_clock::now();
    Real sum_scalar = 0;
    for (int rep = 0; rep < 200; ++rep)
        sum_scalar += Marshal::Kernel::FusedHeatTracePoisson(t, inv_log2, nmax);
    auto t1 = std::chrono::steady_clock::now();

    std::vector<double> soa_out(n_primes, 0.0);
    auto t2 = std::chrono::steady_clock::now();
    double sum_soa = 0;
    for (int rep = 0; rep < 200; ++rep) {
        Marshal::Kernel::FusedHeatTracePoissonSoA(t, inv_log_p.data(), inv_log_p.size(), nmax, soa_out.data());
        for (double v : soa_out) sum_soa += v;
    }
    auto t3 = std::chrono::steady_clock::now();

    const double scalar_sec = std::chrono::duration<double>(t1 - t0).count();
    const double soa_sec = std::chrono::duration<double>(t3 - t2).count();
    std::printf(
        "bench-heat: scalar=%.3e/s soa=%.3e primes/s (nmax=%d n=%d)\n",
        200.0 / scalar_sec, static_cast<double>(n_primes) * 200.0 / soa_sec, nmax, n_primes);
    std::printf("bench-heat: sum_scalar=%.6Le sum_soa=%.6e\n",
                static_cast<long double>(sum_scalar), sum_soa);
    return 0;
}
