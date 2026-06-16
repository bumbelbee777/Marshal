#include "GLnTwistedCoupling.hxx"

#include "LogPrimeGlobal.hxx"

namespace Marshal::Heat::GLn {

std::vector<GLnPrimeBasisEntry> build_gln_prime_basis(const std::vector<int>& primes,
                                                      const GLnTwistedCouplingSpec& spec) {
    std::vector<GLnPrimeBasisEntry> out;
    if (primes.empty()) return out;

    if (spec.rank <= 1) {
        TwistedLogPrimeOperator twisted;
        twisted.local_ops = LogPrimeGlobal::from_primes(primes).operators;
        twisted.coupling_strength = spec.coupling_lambda;
        twisted.coupling_mode = spec.coupling_mode;
        const auto basis = twisted.build_basis(spec.kmax);
        out.reserve(basis.size());
        for (const auto& m : basis) {
            GLnPrimeBasisEntry e;
            e.u = m.u;
            e.weil_weight = m.weil_weight;
            e.prime = (m.prime_idx >= 0 && static_cast<size_t>(m.prime_idx) < primes.size())
                          ? primes[static_cast<size_t>(m.prime_idx)]
                          : 0;
            e.slot = 0;
            out.push_back(e);
        }
        return out;
    }

    const auto blocks = gln_log_prime_blocks(primes, spec.rank);
    for (const auto& b : blocks) {
        for (int k = 1; k <= spec.kmax; ++k) {
            for (int slot = 0; slot < spec.rank; ++slot) {
                GLnPrimeBasisEntry e;
                e.prime = b.prime;
                e.slot = slot;
                e.u = b.block_eigenvalue(k, slot);
                e.weil_weight = b.log_p / static_cast<Real>(spec.rank);
                out.push_back(e);
            }
        }
    }
    return out;
}

}  // namespace Marshal::Heat::GLn
