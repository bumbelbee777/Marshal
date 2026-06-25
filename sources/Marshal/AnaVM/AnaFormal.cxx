#include "AnaFormal.hxx"

#include "FormalCalibration.hxx"
#include "Analysis/PairCorrelation.hxx"
#include <algorithm>
#include <fstream>
#include <iomanip>

namespace Marshal::AnaVM {
namespace {
constexpr Real kPi = 3.141592653589793238462643383279502884L;
}

CountingAnalytics compute_counting_analytics(const std::vector<int>& primes, Real T,
                                             int riemann_count_in_window) {
    CountingAnalytics c;
    c.window_t = T;
    c.riemann_count = riemann_count_in_window;
    for (int p : primes) c.chebyshev_theta += std::log(static_cast<Real>(p));
    c.predicted_cylinder_count = (T / kPi) * c.chebyshev_theta;
    c.counting_diverges = !primes.empty();
    return c;
}

FormalAnalyticsResult run_formal_analytics(const MrsProgram& prog, const OperatorTraits& traits,
                                           const CountingAnalytics& counting,
                                           const Analysis::PairCorrelationReport* pair) {
    FormalAnalyticsResult r;
    r.ansatz_id = prog.id;
    r.traits = traits;
    r.counting = counting;
    r.density_slopes_incompatible = true;

    if (counting.counting_diverges && counting.predicted_cylinder_count > counting.riemann_count)
        r.proved_gates.push_back("cylinder_density_divergence");
    else
        r.failed_gates.push_back("cylinder_density_divergence");

    if (traits.space == SpaceKind::CircleLogp && traits.merge == SpectrumMerge::DirectSum &&
        !traits.placeholder)
        r.proved_gates.push_back("density_slope_mismatch");
    else
        r.diagnostic_gates.push_back("density_slope_mismatch");

    if (pair) {
        r.gue_spacing_l2_cylinder = pair->gue_spacing_l2_cylinder;
        r.gue_spacing_l2_zero = pair->gue_spacing_l2_zero;
        r.montgomery_r2_l2 = pair->montgomery_r2_l2;
        r.pair_correlation_separates = pair->separates_from_gue;
        if (pair->separates_from_gue)
            r.proved_gates.push_back("pair_correlation_vs_gue");
        else
            r.failed_gates.push_back("pair_correlation_vs_gue");
        if (pair->zeros_gue_like) r.diagnostic_gates.push_back("zeros_gue_spacing");
    }

    if (traits.scaffold || traits.placeholder) {
        r.diagnostic_gates.push_back("scaffold_no_identification");
        r.cylinder_class_excluded = false;
    } else if (traits.space == SpaceKind::CircleLogp &&
               traits.scale == SpectrumScale::Omega2PiNOverLogp) {
        r.cylinder_class_excluded = true;
        r.proved_gates.push_back("cylinder_class_membership");
    }

    if (!traits.violated_requirements.empty()) {
        for (const auto& v : traits.violated_requirements) {
            if (r.cylinder_class_excluded)
                r.diagnostic_gates.push_back(v);
            else
                r.failed_gates.push_back(v);
        }
    }

    const bool counting_ok =
        std::find(r.proved_gates.begin(), r.proved_gates.end(), "cylinder_density_divergence") !=
        r.proved_gates.end();
    if (traits.scaffold || traits.placeholder)
        r.mrs_emit_ready = r.failed_gates.empty();
    else if (r.cylinder_class_excluded)
        r.mrs_emit_ready = counting_ok && r.failed_gates.empty();
    else
        r.mrs_emit_ready = false;
    return r;
}

void export_formal_analytics_json(const std::string& path, const FormalAnalyticsResult& r,
                                  const FormalCalibration& cal) {
    std::ofstream out(path);
    out << std::setprecision(17);
    out << "{\n  \"version\": 1,\n  \"engine\": \"AnaVM_FormalAnalytics\",\n";
    out << "  \"ansatz_id\": \"" << r.ansatz_id << "\",\n";
    out << "  \"mrs_emit_ready\": " << (r.mrs_emit_ready ? "true" : "false") << ",\n";
    out << "  \"mrs_module\": \"" << cal.mrs_module << "\",\n";
    out << "  \"cylinder_class_excluded\": " << (r.cylinder_class_excluded ? "true" : "false")
        << ",\n";
    out << "  \"counting\": {\n";
    out << "    \"window_t\": " << static_cast<double>(r.counting.window_t) << ",\n";
    out << "    \"predicted_cylinder_count\": "
        << static_cast<double>(r.counting.predicted_cylinder_count) << ",\n";
    out << "    \"riemann_count\": " << r.counting.riemann_count << ",\n";
    out << "    \"chebyshev_theta\": " << static_cast<double>(r.counting.chebyshev_theta) << "\n";
    out << "  },\n";
    out << "  \"pair_correlation\": {\n";
    out << "    \"gue_spacing_l2_zero\": " << static_cast<double>(r.gue_spacing_l2_zero) << ",\n";
    out << "    \"gue_spacing_l2_cylinder\": "
        << static_cast<double>(r.gue_spacing_l2_cylinder) << ",\n";
    out << "    \"montgomery_r2_l2\": " << static_cast<double>(r.montgomery_r2_l2) << ",\n";
    out << "    \"separates_from_gue\": " << (r.pair_correlation_separates ? "true" : "false")
        << "\n  },\n";
    auto emit_list = [&](const char* key, const std::vector<std::string>& xs) {
        out << "  \"" << key << "\": [";
        for (size_t i = 0; i < xs.size(); ++i) {
            if (i) out << ", ";
            out << "\"" << xs[i] << "\"";
        }
        out << "],\n";
    };
    emit_list("proved_gates", r.proved_gates);
    emit_list("diagnostic_gates", r.diagnostic_gates);
    emit_list("failed_gates", r.failed_gates);
    out << "  \"traits\": {\n";
    out << "    \"space\": \"" << space_kind_string(r.traits.space) << "\",\n";
    out << "    \"merge\": \"" << spectrum_merge_string(r.traits.merge) << "\",\n";
    out << "    \"uses_gamma\": " << (r.traits.uses_gamma ? "true" : "false") << "\n";
    out << "  }\n}\n";
}

}  // namespace Marshal::AnaVM
