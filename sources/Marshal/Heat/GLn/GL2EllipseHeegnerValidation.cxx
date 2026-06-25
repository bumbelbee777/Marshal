#include "GL2EllipseHeegnerValidation.hxx"

#include "GLnArchimedeanOperator.hxx"
#include "GLnLadderMetrics.hxx"
#include "MarshalGLnDirac.hxx"
#include "CombinedConnesDirac.hxx"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>

namespace Marshal::Heat::GLn {

namespace {

constexpr Real kKernelTol = 1e-6L;
constexpr int kMajorMaassLevels = 6;

MarshalGLnDiracSpec ellipse_spec_from_config(const Config& cfg) {
    CombinedConnesDiracSpec cspec;
    cspec.bk = spec_from_config(cfg);
    cspec.theta = cspec.bk.theta > 0 ? cspec.bk.theta : 5.759586531581287L;
    cspec.coupling_lambda = 0;
    cspec.kmax = cfg.kmax > 0 ? std::min(cfg.kmax, 8) : 8;
    MarshalGLnDiracSpec gln = marshal_gln_spec_from_combined(cspec);
    gln.rank = 2;
    gln.arch.rank = 2;
    gln.arch.preset = GLnArchPreset::MaassEllipseHeegner;
    gln.arch.theta = cspec.theta;
    gln.arch.arch_cap = 40;
    gln.coupling.kmax = cspec.kmax;
    gln.coupling.coupling_lambda = 0;
    return gln;
}

}  // namespace

GL2EllipseHeegnerReport run_gl2_ellipse_heegner_validation(const Config& cfg,
                                                             const std::vector<int>& primes) {
    GL2EllipseHeegnerReport rep;
    std::vector<int> sub = primes;
    if (static_cast<int>(sub.size()) > 40) sub.resize(40);

    const auto gln = ellipse_spec_from_config(cfg);
    rep.theta = gln.arch.theta;
    rep.major_arc_threshold = cfg.bound_audit.major_arc_threshold > 0
                                  ? cfg.bound_audit.major_arc_threshold
                                  : 0.45L;
    rep.minor_arc_ub =
        cfg.bound_audit.minor_arc_ub > 0 ? cfg.bound_audit.minor_arc_ub : 0.01L;
    rep.kernel_tolerance = kKernelTol;

    const auto arch_ladder = build_gln_archimedean_ladder(gln.arch);
    const auto result = build_gln_dirac_spectrum(gln, sub);
    rep.kernel_multiplicity = ladder_count_kernel(arch_ladder, kKernelTol);
    rep.heegner_point_count = rep.kernel_multiplicity;

    rep.major_arc_spectral_mass =
        ladder_maass_major_arc_mass(arch_ladder, rep.theta, kKernelTol, kMajorMaassLevels);
    rep.minor_arc_bound =
        ladder_maass_minor_arc_tail_bound(rep.theta, kMajorMaassLevels);

    rep.major_arc_ok = rep.major_arc_spectral_mass >= rep.major_arc_threshold;
    rep.minor_arc_ok = rep.minor_arc_bound < rep.minor_arc_ub;
    rep.goldbach_shared_gln2_ok = rep.kernel_multiplicity >= 1;
    rep.bounds_ok = rep.major_arc_ok && rep.minor_arc_ok && rep.goldbach_shared_gln2_ok;
    rep.proof_status = "PENDING";
    (void)result;
    return rep;
}

bool export_gl2_ellipse_heegner_json(const std::string& path, const GL2EllipseHeegnerReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"version\": 1,\n";
    out << "  \"rank\": " << r.rank << ",\n";
    out << "  \"theta\": " << static_cast<double>(r.theta) << ",\n";
    out << "  \"heegner_point_count\": " << r.heegner_point_count << ",\n";
    out << "  \"kernel_multiplicity\": " << r.kernel_multiplicity << ",\n";
    out << "  \"major_arc_spectral_mass\": " << static_cast<double>(r.major_arc_spectral_mass)
        << ",\n";
    out << "  \"minor_arc_bound\": " << static_cast<double>(r.minor_arc_bound) << ",\n";
    out << "  \"major_arc_threshold\": " << static_cast<double>(r.major_arc_threshold) << ",\n";
    out << "  \"minor_arc_ub\": " << static_cast<double>(r.minor_arc_ub) << ",\n";
    out << "  \"kernel_tolerance\": " << static_cast<double>(r.kernel_tolerance) << ",\n";
    out << "  \"bounds_ok\": " << (r.bounds_ok ? "true" : "false") << ",\n";
    out << "  \"goldbach_shared_gln2_ok\": " << (r.goldbach_shared_gln2_ok ? "true" : "false")
        << ",\n";
    out << "  \"major_arc_ok\": " << (r.major_arc_ok ? "true" : "false") << ",\n";
    out << "  \"minor_arc_ok\": " << (r.minor_arc_ok ? "true" : "false") << ",\n";
    out << "  \"proof_status\": \"" << r.proof_status << "\"\n}\n";
    return true;
}

}  // namespace Marshal::Heat::GLn
