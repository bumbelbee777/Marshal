#include "GL2GoldbachEffectiveValidation.hxx"

#include <algorithm>
#include <vector>

namespace Marshal::Heat::GLn {

namespace {

std::vector<bool> sieve_primes(int limit) {
    std::vector<bool> is_prime(static_cast<size_t>(limit + 1), true);
    if (limit >= 0) is_prime[0] = false;
    if (limit >= 1) is_prime[1] = false;
    for (int p = 2; p * p <= limit; ++p) {
        if (!is_prime[static_cast<size_t>(p)]) continue;
        for (int m = p * p; m <= limit; m += p) is_prime[static_cast<size_t>(m)] = false;
    }
    return is_prime;
}

bool even_n_is_goldbach(int n, const std::vector<bool>& is_prime) {
    if (n < 4 || (n % 2) != 0) return false;
    for (int p = 2; p <= n / 2; ++p) {
        if (p >= static_cast<int>(is_prime.size())) break;
        if (is_prime[static_cast<size_t>(p)] && is_prime[static_cast<size_t>(n - p)]) return true;
    }
    return false;
}

}  // namespace

GoldbachEffectiveReport run_goldbach_effective_validation(int n0, int n_max) {
    GoldbachEffectiveReport rep;
    rep.n0 = std::max(4, n0);
    rep.n_max_checked = std::max(rep.n0, n_max);
    const auto is_prime = sieve_primes(rep.n_max_checked);
    for (int n = rep.n0; n <= rep.n_max_checked; n += 2) {
        ++rep.even_count;
        if (!even_n_is_goldbach(n, is_prime)) {
            rep.failure_n = n;
            rep.ok = false;
            return rep;
        }
    }
    rep.ok = rep.even_count > 0;
    return rep;
}

}  // namespace Marshal::Heat::GLn
