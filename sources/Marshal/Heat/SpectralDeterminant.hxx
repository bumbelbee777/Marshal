#pragma once

#include <string>
#include <vector>

#include "AnaVM/MrsTypes.hxx"
#include "ArchimedeanBoundary.hxx"
#include "Config.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::Heat {

struct PrimeCatalog;

struct SpectralDetPoint {
    Real s_im = 0;
    Real log_det_prime = 0;
    Real log_det_arch = 0;
    Real log_lhs_zeros = 0;
    Real log_weil_rhs = 0;
    Real laplace_duality_gap = 0;
    Real gap = 0;
};

struct SpectralDetBoundaryRow {
    AnaVM::ArchimedeanBoundary boundary = AnaVM::ArchimedeanBoundary::BerryKeating;
    std::string boundary_name;
    Real laplace_weil_residual = 0;
    Real xi_det_gap = 0;
    Real log_det_gap = 0;
    Real t1_gap = 0;
};

struct SpectralDeterminantReport {
    std::string program_id;
    AnaVM::MrsSpectralDeterminant spec;
    AnaVM::ArchimedeanBoundary boundary = AnaVM::ArchimedeanBoundary::BerryKeating;
    std::string boundary_name;
    Real laplace_a = 1.0L;
    Real laplace_weil_residual = 0;
    Real laplace_weil_log_gap = 0;
    Real laplace_weil_log_gap_max = 0;
    Real xi_det_gap = 0;
    Real xi_det_gap_max = 0;
    Real log_det_gap = 0;
    Real t1_gap = 0;
    std::vector<SpectralDetPoint> samples;
    std::vector<SpectralDetBoundaryRow> boundary_sweep;
    int best_boundary_index = -1;
    std::string verdict;
};

SpectralDeterminantReport run_spectral_determinant_validation(
    const Config& cfg, const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    PrimeCatalog& cat, const std::vector<int>& primes);

bool export_spectral_determinant_json(const std::string& path, const SpectralDeterminantReport& r);

}  // namespace Marshal::Heat
