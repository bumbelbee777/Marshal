#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

#include "Marshal/Heat/LogPrimeOperator.hxx"
#include "Marshal/Heat/LogPrimeGlobal.hxx"
#include "Marshal/Numerics/TestFunctions.hxx"

static void ExpectNear(long double got, long double ref, long double rel_tol, const char* msg) {
    const long double denom = std::max(1.0L, std::fabsl(ref));
    if (std::fabsl(got - ref) > rel_tol * denom) {
        std::fprintf(stderr, "FAIL %s: got %.17Lg ref %.17Lg\n", msg, got, ref);
        std::abort();
    }
}

static void TestWeilVsPWeightDiffer() {
    const auto op = Marshal::Heat::LogPrimeOperator::from_prime(7);
    const Marshal::GaussTest gauss(2.23606797749978969641L);
    const int kmax = 30;
    const long double weil = op.weil_prime_sum(gauss, kmax, 1e-30L);
    const long double pw = op.p_weight_sum(gauss, kmax, 1e-30L);
    ExpectNear(weil / op.log_p, pw, 1e-12L, "weil/log_p = p_weight");
    assert(std::fabsl(weil - pw) > 1e-6L);
    std::puts("TestWeilVsPWeightDiffer OK");
}

static void TestGlobalWeilSum() {
    std::vector<int> primes = {2, 3, 5, 7, 11};
    const Marshal::GaussTest gauss(1.0L);
    const auto g = Marshal::Heat::LogPrimeGlobal::from_primes(primes);
    long double sum = 0;
    for (int p : primes) sum += Marshal::Heat::LogPrimeOperator::from_prime(p).weil_prime_sum(gauss, 20, 1e-30L);
    ExpectNear(g.weil_prime_sum(gauss, 20, 1e-30L), sum, 1e-12L, "global weil");
    std::puts("TestGlobalWeilSum OK");
}

int main() {
    TestWeilVsPWeightDiffer();
    TestGlobalWeilSum();
    std::puts("All log-prime unit tests passed.");
    return 0;
}
