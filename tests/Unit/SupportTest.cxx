#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "Marshal/Compat.hxx"
#include "Marshal/Numerics/TestFunctions.hxx"

static void ExpectNear(long double got, long double ref, long double rel_tol, const char* msg) {
    const long double denom = std::max(1.0L, fabsl(ref));
    if (fabsl(got - ref) > rel_tol * denom) {
        std::fprintf(stderr, "FAIL %s: got %.17Lg ref %.17Lg tol %.3Lg\n", msg, got, ref, rel_tol);
        std::abort();
    }
}

static void TestPairwiseSum() {
    ExpectNear(pairwise_sum({1.0L, 2.0L, 3.0L, 4.0L}), 10.0L, 0.0L, "pairwise_sum");
    std::puts("TestPairwiseSum OK");
}

static void TestTailRegime() {
    using Marshal::Numerics::ConvergenceRegime;
    using Marshal::Numerics::TailBoundStatus;
    if (ConvergenceRegime(0.005L, 1e7L)) {
        std::fputs("FAIL ConvergenceRegime short horizon\n", stderr);
        std::abort();
    }
    if (!ConvergenceRegime(0.005L, 1e50L)) {
        std::fputs("FAIL ConvergenceRegime long horizon\n", stderr);
        std::abort();
    }
    const int primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47};
    if (Marshal::Numerics::ClassifyTailBound(0.01L, 30.0L, 0.0L, -1.0L) != TailBoundStatus::Divergent) {
        std::fputs("FAIL ClassifyTailBound divergent\n", stderr);
        std::abort();
    }
    const Real exact = evaluate_omitted_prime_tail(0.01L, 1e50L, primes, 15, 20);
    const Real bound = bound_omitted_prime_tail(0.01L, 1e50L);
    const auto st = ClassifyTailBound(0.01L, 1e50L, exact, bound);
    if (st != TailBoundStatus::Valid && st != TailBoundStatus::Unproved) {
        std::fputs("FAIL ClassifyTailBound regime\n", stderr);
        std::abort();
    }
    std::puts("TestTailRegime OK");
}

static void TestExpPoly() {
#if defined(MARSHAL_HAVE_AVX2)
    for (double x = -20.0; x <= 0.0; x += 0.05) {
        const double ref = std::exp(x);
        const double got = expd1_avx2(x);
        if (std::fabs(got - ref) >= 1e-3 * std::max(1.0, std::fabs(ref))) {
            std::fprintf(stderr, "FAIL TestExpPoly x=%g got=%g ref=%g\n", x, got, ref);
            std::abort();
        }
    }
    std::puts("TestExpPoly OK");
#else
    std::puts("TestExpPoly SKIP");
#endif
}

int main() {
    TestPairwiseSum();
    TestTailRegime();
    TestExpPoly();
    std::puts("All unit tests passed.");
    return 0;
}
