#pragma once

#include "LogPrimeOperator.hxx"
#include "Numerics/Real.hxx"

#include <vector>

namespace Marshal::Heat::GLn {

/// n×n log-prime block per prime (rank=1 → scalar eigenvalues).
struct GLnLogPrimeBlock {
    int rank = 1;
    int prime = 0;
    Real log_p = 0;
    Real sqrt_p = 0;

    static GLnLogPrimeBlock from_prime(int p, int rank);

    /// Eigenvalue of mode k (1-based) on diagonal slot i (0-based).
    Real block_eigenvalue(int k, int slot) const;
};

std::vector<GLnLogPrimeBlock> gln_log_prime_blocks(const std::vector<int>& primes, int rank);

}  // namespace Marshal::Heat::GLn
