#include "InvestigationConfig.hxx"

#include "InvestigationTypes.hxx"

namespace Marshal::Investigation {

void apply_mrs_investigation(Config& cfg, const AnaVM::MrsInvestigation& inv) {
    if (!inv.present) return;
    cfg.investigation_run = true;
    if (!inv.id.empty()) cfg.investigation_id = inv.id;
    if (!inv.cert_root.empty()) cfg.investigation_cert_root = inv.cert_root;
    if (inv.quick) cfg.investigation_quick = true;

    InvestigationSpec& s = cfg.investigation_spec;
    if (!inv.id.empty()) s.id = inv.id;
    if (!inv.cert_root.empty()) s.cert_root = inv.cert_root;
    s.quick = inv.quick;
    s.fixed_theta = static_cast<Real>(inv.fixed_theta);
    s.fixed_boundary = inv.fixed_boundary;
    s.t1_tolerance = static_cast<Real>(inv.t1_tolerance);
    s.heat_scale_base = static_cast<Real>(inv.heat_scale_base);
    s.heat_scales = inv.heat_scales;
    s.coupling_lambda = static_cast<Real>(inv.coupling_lambda);
    s.combined_cap = inv.combined_cap;
    s.arch_cap = inv.arch_cap;
    s.kmax = inv.mode_kmax;
    s.run_curvature = inv.run_curvature;
    s.run_topology = inv.run_topology;
    s.run_heat_trace = inv.run_heat_trace;
    s.run_spacing = inv.run_spacing;
    s.run_continuum = inv.run_continuum;

    s.curvature_sweep.mode = ThetaSweepMode::Centered;
    s.curvature_sweep.center = static_cast<Real>(inv.curvature_center);
    s.curvature_sweep.range = static_cast<Real>(inv.curvature_range);
    s.curvature_sweep.steps = inv.curvature_steps;

    s.topology_sweep.mode = ThetaSweepMode::Uniform;
    s.topology_sweep.min_theta = static_cast<Real>(inv.topology_min);
    s.topology_sweep.max_theta = static_cast<Real>(inv.topology_max);
    s.topology_sweep.steps = inv.topology_steps;

    if (!inv.heat_t_values.empty()) {
        s.heat_t_ladder.param = "heat_t";
        s.heat_t_ladder.values.clear();
        for (double v : inv.heat_t_values) s.heat_t_ladder.values.push_back(static_cast<Real>(v));
    }
    if (!inv.prime_limit_values.empty()) {
        s.prime_limit_ladder.param = "prime_limit";
        s.prime_limit_ladder.values.clear();
        for (int pl : inv.prime_limit_values) s.prime_limit_ladder.values.push_back(static_cast<Real>(pl));
    }
}

}  // namespace Marshal::Investigation
