#include "GenusOneWedgeValidation.hxx"

#include "GenusOneLogBoundsValidation.hxx"

namespace Marshal::Heat {

GenusOneWedgeReport run_genus_one_wedge_validation(const Config& cfg,
                                                   const std::vector<double>& gammas) {
    GenusOneWedgeReport rep;
    const AnaVM::MrsGenusOneLogBounds* genus =
        cfg.genus_one_log_bounds.present ? &cfg.genus_one_log_bounds : nullptr;
    const AnaVM::MrsBoundAudit* bounds = cfg.bound_audit.present ? &cfg.bound_audit : nullptr;
    const auto bounds_rep = run_genus_one_log_bounds_validation(gammas, genus, bounds);
    rep.program_id = cfg.anavm.id.empty() ? "marshal_wedge_analytic" : cfg.anavm.id;
    rep.zero_truncation = bounds_rep.zero_truncation;
    rep.small_z_threshold = bounds_rep.small_z_threshold;
    rep.log_majorant_c = bounds_rep.log_majorant_c;
    rep.log_partial_sum_ub = bounds_rep.log_partial_sum_ub;
    rep.max_log_times_gamma2 = bounds_rep.max_log_times_gamma2;
    rep.max_partial_log_abs_sum = bounds_rep.max_partial_log_abs_sum;
    rep.genus_one_log_summability_ok = bounds_rep.genus_one_log_summability_ok;
    rep.lean_log_emit_ready = bounds_rep.genus_one_log_summability_ok;
    rep.log_tail_rows.reserve(bounds_rep.log_tail_rows.size());
    for (const auto& row : bounds_rep.log_tail_rows) {
        WedgeLogTailRow out;
        out.s_re = row.s_re;
        out.s_im = row.s_im;
        out.tail_start_n = row.tail_start_n;
        out.partial_log_abs_sum = row.partial_log_abs_sum;
        out.partial_inv_gamma2_sum = row.partial_inv_gamma2_sum;
        out.max_log_times_gamma2 = row.max_log_times_gamma2;
        out.tail_count = row.tail_count;
        rep.log_tail_rows.push_back(out);
    }
    rep.note =
        "C++ genus-1 log bounds via AnaVM/MRS (`genus_one_log_bounds`); Lean cert in "
        "MarshalXiHadamardAnaVmCert + MarshalAnaVmGenusOneLogBounds.";
    return rep;
}

bool export_genus_one_wedge_json(const std::string& path, const GenusOneWedgeReport& r) {
    GenusOneLogBoundsReport br;
    br.program_id = r.program_id;
    br.zero_truncation = r.zero_truncation;
    br.small_z_threshold = r.small_z_threshold;
    br.log_majorant_c = r.log_majorant_c;
    br.log_partial_sum_ub = r.log_partial_sum_ub;
    br.max_log_times_gamma2 = r.max_log_times_gamma2;
    br.max_partial_log_abs_sum = r.max_partial_log_abs_sum;
    br.genus_one_log_summability_ok = r.genus_one_log_summability_ok;
    for (const auto& row : r.log_tail_rows) {
        GenusOneLogTailRow out;
        out.s_re = row.s_re;
        out.s_im = row.s_im;
        out.tail_start_n = row.tail_start_n;
        out.partial_log_abs_sum = row.partial_log_abs_sum;
        out.partial_inv_gamma2_sum = row.partial_inv_gamma2_sum;
        out.max_log_times_gamma2 = row.max_log_times_gamma2;
        out.tail_count = row.tail_count;
        out.ok = r.genus_one_log_summability_ok;
        br.log_tail_rows.push_back(out);
    }
    return export_genus_one_log_bounds_json(path, br);
}

}  // namespace Marshal::Heat
