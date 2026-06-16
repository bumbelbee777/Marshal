#pragma once

#include "ConnesCouplingMode.hxx"
#include "GLnLogPrimeBlock.hxx"
#include "Numerics/Real.hxx"
#include "TwistedLogPrimeOperator.hxx"

#include <vector>

namespace Marshal::Heat::GLn {

struct GLnTwistedCouplingSpec {
    int rank = 1;
    Real coupling_lambda = 0.5L;
    ConnesCouplingMode coupling_mode = ConnesCouplingMode::LogLadder;
    int kmax = 12;
};

/// Block-structured Q× coupling basis (rank=1 delegates to TwistedLogPrimeOperator).
struct GLnPrimeBasisEntry {
    Real u = 0;
    Real weil_weight = 0;
    int prime = 0;
    int slot = 0;
};

std::vector<GLnPrimeBasisEntry> build_gln_prime_basis(const std::vector<int>& primes,
                                                      const GLnTwistedCouplingSpec& spec);

}  // namespace Marshal::Heat::GLn
