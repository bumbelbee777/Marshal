#include <cmath>
#include <cstdio>
#include <vector>

#include "Marshal/Kernel/FusedHotPaths.hxx"

static void TestZeroGaussianBatch() {
    const std::vector<double> gamma = {14.134725f, 21.022040f, 25.010858f, 30.424876f};
    const std::vector<double> t_vals = {0.005, 0.01, 0.02, 0.05};
    std::vector<double> batched(t_vals.size(), 0.0);
    Marshal::Kernel::FusedZeroGaussianSumBatch(
        gamma.data(), gamma.size(), t_vals.data(), t_vals.size(), batched.data());

    for (size_t j = 0; j < t_vals.size(); ++j) {
#if defined(MARSHAL_HAVE_AVX2)
        const double ref = Marshal::Kernel::FusedZeroGaussianSum4(
            gamma.data(), gamma.size(), t_vals[j]);
#else
        double sum = 0.0;
        const double t = t_vals[j];
        for (double g : gamma)
            sum += 2.0 * std::exp(-t * g * g);
        const double ref = sum;
#endif
        const double rel = std::fabs(batched[j] - ref) / std::max(1.0, std::fabs(ref));
        if (rel >= 1e-15) {
            std::fprintf(stderr, "batch mismatch j=%zu got=%.17g ref=%.17g rel=%.3e\n",
                         j, batched[j], ref, rel);
            std::abort();
        }
    }
    std::puts("TestZeroGaussianBatch OK");
}

static void TestPoissonSoA() {
    const std::vector<double> inv_log_p = {1.0 / std::log(2.0), 1.0 / std::log(3.0), 1.0 / std::log(5.0)};
    constexpr int nmax = 32;
    const Real t = 0.01L;
    std::vector<double> batched(inv_log_p.size(), 0.0);
    Marshal::Kernel::FusedHeatTracePoissonSoA(
        t, inv_log_p.data(), inv_log_p.size(), nmax, batched.data());

    for (size_t i = 0; i < inv_log_p.size(); ++i) {
        const double ref = static_cast<double>(
            Marshal::Kernel::FusedHeatTracePoisson(t, static_cast<Real>(inv_log_p[i]), nmax));
        const double rel = std::fabs(batched[i] - ref) / std::max(1.0, std::fabs(ref));
        if (rel >= 1e-14) {
            std::fprintf(stderr, "poisson SoA mismatch i=%zu got=%.17g ref=%.17g rel=%.3e\n",
                         i, batched[i], ref, rel);
            std::abort();
        }
    }
    std::puts("TestPoissonSoA OK");
}

static void TestWeilBlockSoA() {
    const std::vector<double> log_p = {std::log(2.0), std::log(3.0), std::log(5.0)};
    const std::vector<int> k_cap = {8, 5, 4};
    const Real t = 0.01L;
    std::vector<double> batched(log_p.size(), 0.0);
    Marshal::Kernel::FusedPrimeBlockSoA(
        t, log_p.data(), k_cap.data(), log_p.size(), batched.data());

    for (size_t i = 0; i < log_p.size(); ++i) {
        const double ref = static_cast<double>(
            Marshal::Kernel::FusedPrimeBlock(t, static_cast<Real>(log_p[i]), k_cap[i]));
        const double rel = std::fabs(batched[i] - ref) / std::max(1.0, std::fabs(ref));
        if (rel >= 1e-14) {
            std::fprintf(stderr, "weil block SoA mismatch i=%zu got=%.17g ref=%.17g rel=%.3e\n",
                         i, batched[i], ref, rel);
            std::abort();
        }
    }
    std::puts("TestWeilBlockSoA OK");
}

int main() {
    TestZeroGaussianBatch();
    TestPoissonSoA();
    TestWeilBlockSoA();
    std::puts("All batch kernel tests passed.");
    return 0;
}
