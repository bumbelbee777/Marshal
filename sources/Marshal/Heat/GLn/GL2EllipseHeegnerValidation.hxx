#pragma once

#include "Config.hxx"
#include "Numerics/Real.hxx"

#include <string>
#include <vector>

namespace Marshal::Heat::GLn {

struct GL2EllipseHeegnerReport {
    int rank = 2;
    Real theta = 0;
    int heegner_point_count = 0;
    int kernel_multiplicity = 0;
    Real major_arc_spectral_mass = 0;
    Real minor_arc_bound = 0;
    Real major_arc_threshold = 0.45L;
    Real minor_arc_ub = 0.01L;
    Real kernel_tolerance = 1e-6L;
    bool bounds_ok = false;
    bool goldbach_shared_gln2_ok = false;
    bool major_arc_ok = false;
    bool minor_arc_ok = false;
    std::string proof_status = "PENDING";
};

GL2EllipseHeegnerReport run_gl2_ellipse_heegner_validation(const Config& cfg,
                                                             const std::vector<int>& primes);

bool export_gl2_ellipse_heegner_json(const std::string& path, const GL2EllipseHeegnerReport& r);

}  // namespace Marshal::Heat::GLn
