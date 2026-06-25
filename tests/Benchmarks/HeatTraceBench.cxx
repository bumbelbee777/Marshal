#include <chrono>
#include <cstdio>
#include <vector>
#include "Marshal/Compat.hxx"
#include "Marshal/Kernel/FusedHotPaths.hxx"
#include "Marshal/Kernel/ScaleHotPaths.hxx"
#include "Marshal/Kernel/SoaStreaming.hxx"

int main() {
    const Real t = 0.01L;
    const Real inv_log2 = 1.0L / std::log(2.0L);
    constexpr int nmax = 5000;
    constexpr int n_primes = 4096;
    constexpr int n_tau = 32;

    std::vector<double> inv_log_p(n_primes);
    std::vector<double> log_p(n_primes);
    std::vector<int> k_cap(n_primes, nmax);
    for (int i = 0; i < n_primes; ++i) {
        log_p[static_cast<size_t>(i)] = std::log(static_cast<double>(i + 2));
        inv_log_p[static_cast<size_t>(i)] = 1.0 / log_p[static_cast<size_t>(i)];
    }

    std::vector<double> tau_values(n_tau);
    for (int j = 0; j < n_tau; ++j)
        tau_values[static_cast<size_t>(j)] = 0.005 + 0.001 * static_cast<double>(j);

    auto t0 = std::chrono::steady_clock::now();
    Real sum_scalar = 0;
    for (int rep = 0; rep < 200; ++rep)
        sum_scalar += Marshal::Kernel::FusedHeatTracePoisson(t, inv_log2, nmax);
    auto t1 = std::chrono::steady_clock::now();

    std::vector<double> poisson_out(n_primes, 0.0);
    auto t2 = std::chrono::steady_clock::now();
    double sum_soa = 0;
    for (int rep = 0; rep < 200; ++rep) {
        Marshal::Kernel::FusedHeatTracePoissonSoA(t, inv_log_p.data(), inv_log_p.size(), nmax,
                                                  poisson_out.data());
        for (double v : poisson_out) sum_soa += v;
    }
    auto t3 = std::chrono::steady_clock::now();

    std::vector<double> prime_out(n_primes, 0.0);
    auto t4 = std::chrono::steady_clock::now();
    double sum_prime = 0;
    for (int rep = 0; rep < 200; ++rep) {
        Marshal::Kernel::FusedPrimeBlockSoA(t, log_p.data(), k_cap.data(), n_primes,
                                            prime_out.data());
        for (double v : prime_out) sum_prime += v;
    }
    auto t5 = std::chrono::steady_clock::now();

    std::vector<double> heat_batch(static_cast<size_t>(n_tau) * n_primes, 0.0);
    std::vector<Real> log_p_ld(n_primes);
    for (int i = 0; i < n_primes; ++i)
        log_p_ld[static_cast<size_t>(i)] = static_cast<Real>(log_p[static_cast<size_t>(i)]);
    auto t6 = std::chrono::steady_clock::now();
    double sum_batch = 0;
    for (int rep = 0; rep < 50; ++rep) {
        Marshal::Kernel::FusedAbHeatBlockBatch(log_p_ld.data(), k_cap.data(), n_primes, 0,
                                               tau_values.data(), tau_values.size(),
                                               heat_batch.data());
        for (double v : heat_batch) sum_batch += v;
    }
    auto t7 = std::chrono::steady_clock::now();

    const double scalar_sec = std::chrono::duration<double>(t1 - t0).count();
    const double soa_sec = std::chrono::duration<double>(t3 - t2).count();
    const double prime_sec = std::chrono::duration<double>(t5 - t4).count();
    const double batch_sec = std::chrono::duration<double>(t7 - t6).count();
    std::printf(
        "bench-heat: poisson_scalar=%.3e/s poisson_soa=%.3e primes/s "
        "prime_soa=%.3e primes/s heat_batch=%.3e cells/s\n",
        200.0 / scalar_sec, static_cast<double>(n_primes) * 200.0 / soa_sec,
        static_cast<double>(n_primes) * 200.0 / prime_sec,
        static_cast<double>(n_primes) * n_tau * 50.0 / batch_sec);
    std::printf("bench-heat: sums scalar=%.6Le soa=%.6e prime=%.6e batch=%.6e\n",
                static_cast<long double>(sum_scalar), sum_soa, sum_prime, sum_batch);
    return 0;
}
