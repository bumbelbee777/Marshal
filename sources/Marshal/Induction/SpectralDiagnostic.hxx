#pragma once
#include <vector>
#include "Config.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/TestFunctions.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::Induction {

struct CompactSinc2Result {
    Real residual = 0;
    Real T = 0;
    Real lhs = 0;
    Real rhs = 0;
    bool mismatch_proved = false;
};

constexpr Real kCompactSinc2MismatchTol = 1e-10L;

void FillLexSortedGaps(const std::vector<Real>& sorted_omegas, const std::vector<double>& gammas,
                       size_t m, Real& max_gap, Real& mean_gap);

void FillMatchedCylinderGaps(const Heat::PrimeCatalog& cat, const std::vector<double>& gammas,
                             size_t m, int k_primes, Real& max_gap, Real& mean_gap,
                             Real& max_sq_gap, Real& mean_sq_gap);

void FillFixedModeGaps(const Heat::PrimeCatalog& cat, const std::vector<double>& gammas, size_t m,
                       int k_primes, Real& max_gap, Real& mean_gap);

void FillExponentGaps(const Heat::PrimeCatalog& cat, const std::vector<double>& gammas, size_t m,
                      int k_primes, int kmax, Real& max_gap, Real& mean_gap, Real& max_sq_gap,
                      Real& mean_sq_gap);

void FillFixedQuotientGaps(const Heat::PrimeCatalog& cat, const std::vector<double>& gammas,
                           size_t m, int k_primes, Real& max_gap, Real& max_sq_gap);

CompactSinc2Result RunCompactSinc2Falsification(const std::vector<double>& gammas,
                                                const std::vector<Real>& gammas_ld,
                                                Heat::PrimeCatalog& cat, const Config& cfg,
                                                Real T, Real mismatch_tol);

CompactSinc2Result RunQuotientLhsSinc2(const std::vector<double>& gammas,
                                       const std::vector<Real>& gammas_ld,
                                       Heat::PrimeCatalog& cat, const Config& cfg, Real T,
                                       int n_quotient);

}  // namespace Marshal::Induction
