#include "GLnLogPrimeBlock.hxx"

#include <cmath>

namespace Marshal::Heat::GLn {

GLnLogPrimeBlock GLnLogPrimeBlock::from_prime(int p, int rank) {
    GLnLogPrimeBlock b;
    b.rank = std::max(1, rank);
    b.prime = p;
    b.log_p = std::log(static_cast<Real>(p));
    b.sqrt_p = std::sqrt(static_cast<Real>(p));
    return b;
}

Real GLnLogPrimeBlock::block_eigenvalue(int k, int slot) const {
    const Real base = static_cast<Real>(k) * log_p;
    if (rank <= 1) return base;
    const Real slot_shift = static_cast<Real>(slot) * log_p / static_cast<Real>(rank);
    return base + slot_shift;
}

std::vector<GLnLogPrimeBlock> gln_log_prime_blocks(const std::vector<int>& primes, int rank) {
    std::vector<GLnLogPrimeBlock> out;
    out.reserve(primes.size());
    for (int p : primes) out.push_back(GLnLogPrimeBlock::from_prime(p, rank));
    return out;
}

}  // namespace Marshal::Heat::GLn
