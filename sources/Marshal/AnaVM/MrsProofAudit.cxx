#include "MrsProofAudit.hxx"

#include <fstream>
#include <sstream>

namespace Marshal::AnaVM {
namespace {

bool witness_genus_one_log_summability(const MrsHadamardWitnessContext& ctx,
                                       std::string* evidence, std::string* fail) {
    if (!ctx.report) {
        if (fail) *fail = "missing report";
        return false;
    }
    const auto& r = *ctx.report;
    if (!ctx.log_ok || !r.genus_one_log_summability_ok) {
        if (fail) *fail = "genus_one_log_summability_ok=false";
        return false;
    }
    if (r.max_partial_log_abs_sum > r.log_partial_sum_ub) {
        if (fail) *fail = "partial log sum exceeds ub";
        return false;
    }
    std::ostringstream oss;
    oss << "max |log|*gamma^2=" << r.max_log_times_gamma2
        << " partial_log_sum=" << r.max_partial_log_abs_sum;
    if (evidence) *evidence = oss.str();
    return true;
}

bool witness_off_height_log_summability(const MrsHadamardWitnessContext& ctx, std::string* evidence,
                                        std::string* fail) {
    if (!ctx.rh_audit) {
        if (fail) *fail = "missing rh_audit";
        return false;
    }
    if (!ctx.rh_audit->off_height_log_summability_ok) {
        if (fail) *fail = "off_height_log_summability witness failed";
        return false;
    }
    if (evidence) *evidence = "mrs_witness:off_height_log_summability";
    return true;
}

bool witness_certified_det_eq(const MrsHadamardWitnessContext& ctx, std::string* evidence,
                              std::string* fail) {
    if (!ctx.report || !ctx.ident_ok || !ctx.grid_ok) {
        if (fail) *fail = "wedge calibration / grid identification failed";
        return false;
    }
    std::ostringstream oss;
    oss << "max_ident_gap_decades=" << ctx.report->max_ident_gap_decades;
    if (evidence) *evidence = oss.str();
    return true;
}

bool witness_grid_pointwise(const MrsHadamardWitnessContext& ctx, std::string* evidence,
                            std::string* fail) {
    if (!ctx.report || !ctx.grid_ok) {
        if (fail) *fail = "grid_pointwise_identification_ok=false";
        return false;
    }
    std::ostringstream oss;
    oss << "max_grid_rel=" << ctx.report->max_grid_rel_gap
        << " max_tail_decades=" << ctx.report->max_tail_bound_decades;
    if (evidence) *evidence = oss.str();
    return true;
}

bool witness_holomorphy_gap(const MrsHadamardWitnessContext& ctx, std::string* evidence,
                            std::string* fail) {
    if (!ctx.report || !ctx.holo_ok) {
        if (fail) *fail = "holomorphy_uniform_cauchy_ok=false";
        return false;
    }
    std::ostringstream oss;
    oss << "max_holo_uniform_gap=" << ctx.report->max_holomorphy_uniform_gap;
    if (evidence) *evidence = oss.str();
    return true;
}

bool witness_log_prereq(const MrsHadamardWitnessContext& ctx, std::string* evidence,
                      std::string* fail) {
    if (!ctx.log_ok) {
        if (fail) *fail = "log summability prerequisite failed";
        return false;
    }
    if (evidence) *evidence = "mrs_witness:log_summability";
    return true;
}

bool witness_holo_grid_prereq(const MrsHadamardWitnessContext& ctx, std::string* evidence,
                              std::string* fail) {
    if (!ctx.grid_ok || !ctx.holo_ok) {
        if (fail) *fail = "grid/holomorphy prerequisite failed";
        return false;
    }
    if (evidence) *evidence = "mrs_witness:grid_holomorphy_chain";
    return true;
}

bool witness_wedge_chain(const MrsHadamardWitnessContext& ctx, std::string* evidence,
                         std::string* fail) {
    if (!ctx.log_ok) {
        if (fail) *fail = "log summability prerequisite failed";
        return false;
    }
    if (!ctx.grid_ok || !ctx.holo_ok) {
        if (fail) *fail = "grid/holomorphy prerequisite failed";
        return false;
    }
    if (evidence) *evidence = "mrs_witness:wedge_analytic_chain";
    return true;
}

bool witness_weierstrass_ident(const MrsHadamardWitnessContext& ctx, std::string* evidence,
                                std::string* fail) {
    if (!witness_wedge_chain(ctx, evidence, fail)) return false;
    if (!ctx.ident_ok) {
        if (fail) *fail = "ident calibration failed";
        return false;
    }
    if (ctx.rh_audit && evidence) {
        std::ostringstream oss;
        oss << "wedge_ident max_gap=" << ctx.rh_audit->max_off_forced_rel_gap;
        *evidence = oss.str();
    }
    return true;
}

bool witness_xi_zero_classification(const MrsHadamardWitnessContext& ctx,
                                    const MrsCompilationBundle& bundle,
                                    const std::string& prove_ref, std::string* evidence,
                                    std::string* fail) {
    if (!bundle_has_prove_ref(bundle, prove_ref)) {
        if (fail) *fail = "missing mrs prove ref: " + prove_ref;
        return false;
    }
    if (!ctx.rh_audit || !ctx.rh_audit->xi_zero_classification_ok) {
        if (fail) *fail = "xi_zero_classification witness failed";
        return false;
    }
    if (evidence) *evidence = "mrs_prove:" + prove_ref;
    return true;
}

bool witness_classical_rh(const MrsHadamardWitnessContext& ctx, const MrsCompilationBundle& bundle,
                          const std::string& prove_ref, std::string* evidence,
                          std::string* fail) {
    if (!bundle_has_prove_ref(bundle, prove_ref)) {
        if (fail) *fail = "missing mrs prove ref: " + prove_ref;
        return false;
    }
    if (!ctx.rh_audit || !ctx.rh_audit->classical_rh_ok) {
        if (fail) *fail = "classical RH capstone witness failed";
        return false;
    }
    std::ostringstream oss;
    oss << "mrs_prove:" << prove_ref << " zero_audit_n=" << ctx.rh_audit->rh_zero_audit_count;
    if (evidence) *evidence = oss.str();
    return true;
}

bool evaluate_obligation_witness(const std::string& id, const MrsProofObligationDecl& ob,
                                   const MrsCompilationBundle& bundle,
                                   const MrsHadamardWitnessContext& ctx, std::string* evidence,
                                   std::string* fail) {
    if (id == "genus_one_log_summability")
        return witness_genus_one_log_summability(ctx, evidence, fail);
    if (id == "marshal_off_height_log_summability")
        return witness_off_height_log_summability(ctx, evidence, fail);
    if (id == "certified_det_eq_riemannXi_off_locus")
        return witness_certified_det_eq(ctx, evidence, fail);
    if (id == "grid_pointwise_tprod_eq_xi") return witness_grid_pointwise(ctx, evidence, fail);
    if (id == "holomorphy_uniform_cauchy_gap") return witness_holomorphy_gap(ctx, evidence, fail);
    if (id == "tprod_convergent_off_locus") return witness_log_prereq(ctx, evidence, fail);
    if (id == "wedge_holomorphy_tprod") {
        if (!witness_log_prereq(ctx, evidence, fail)) return false;
        if (!ctx.holo_ok) {
            if (fail) *fail = "holomorphy prerequisite failed";
            return false;
        }
        if (evidence) *evidence = "uniform Cauchy gap + log-tsum holomorphy route";
        return true;
    }
    if (id == "grid_pointwise_tprod_eq_certified") {
        if (!witness_grid_pointwise(ctx, evidence, fail)) return false;
        if (!ctx.ident_ok) {
            if (fail) *fail = "certified det calibration failed";
            return false;
        }
        return true;
    }
    if (id == "wedge_holomorphy_certified") {
        if (!ctx.holo_ok || !ctx.ident_ok) {
            if (fail) *fail = "wedge_holomorphy_certified witness failed";
            return false;
        }
        if (evidence) *evidence = "mrs_witness:spectralDet_holomorphic_on_wedge";
        return true;
    }
    if (id == "identity_theorem_on_wedge") return witness_holo_grid_prereq(ctx, evidence, fail);
    if (id == "strip_extension_via_approach_sequence") {
        if (!bundle_has_prove_ref(bundle, ob.prove_ref)) {
            if (fail) *fail = "missing prove ref " + ob.prove_ref;
            return false;
        }
        if (!witness_holo_grid_prereq(ctx, evidence, fail)) return false;
        if (evidence) *evidence = "mrs_prove:" + ob.prove_ref;
        return true;
    }
    if (id == "wedge_proportionality_from_holomorphy") {
        if (!bundle_has_prove_ref(bundle, ob.prove_ref)) {
            if (fail) *fail = "missing prove ref " + ob.prove_ref;
            return false;
        }
        if (!witness_holo_grid_prereq(ctx, evidence, fail)) return false;
        if (evidence) *evidence = "mrs_prove:" + ob.prove_ref;
        return true;
    }
    if (id == "MarshalHadamardWeierstrassIdentification") {
        if (!bundle_has_prove_ref(bundle, ob.prove_ref)) {
            if (fail) *fail = "missing prove ref " + ob.prove_ref;
            return false;
        }
        if (!witness_weierstrass_ident(ctx, evidence, fail)) return false;
        if (evidence) *evidence = "mrs_prove:" + ob.prove_ref + ";" + *evidence;
        return true;
    }
    if (id == "marshal_xi_zero_classification")
        return witness_xi_zero_classification(ctx, bundle, ob.prove_ref, evidence, fail);
    if (id == "classical_riemann_hypothesis_marshal")
        return witness_classical_rh(ctx, bundle, ob.prove_ref, evidence, fail);
    if (ob.proof_class == ProofClass::Composition && ob.prove_ref.empty()) return true;
    if (fail) *fail = "no witness registered for " + id;
    return false;
}

const MrsProofObligationDecl* find_obligation_decl(const MrsCompilationBundle& bundle,
                                                   const std::string& id) {
    for (const auto& m : bundle.merged_modules) {
        for (const auto& g : m.proof_graphs) {
            for (const auto& ob : g.obligations) {
                if (ob.id == id) return &ob;
            }
        }
    }
    return nullptr;
}

}  // namespace

bool bundle_has_prove_ref(const MrsCompilationBundle& bundle, const std::string& prove_ref) {
    if (prove_ref.empty()) return false;
    for (const auto& m : bundle.merged_modules) {
        for (const auto& p : m.proves) {
            if (p.name == prove_ref) return true;
        }
    }
    return false;
}

MrsProofAuditReport apply_mrs_hadamard_proof_audit(ProofGraphReport& graph,
                                                   const MrsCompilationBundle& bundle,
                                                   const MrsHadamardWitnessContext& ctx) {
    MrsProofAuditReport rep;
    rep.ok = true;

    for (auto& o : graph.obligations) {
        const MrsProofObligationDecl* decl = find_obligation_decl(bundle, o.id);
        const MrsProofObligationDecl ob = decl ? *decl : MrsProofObligationDecl{};

        if (o.proof_class == ProofClass::Composition &&
            ob.prove_kind != MrsProofBodyKind::Infer && ob.prove_ref.empty()) {
            continue;
        }

        MrsProofAuditEntry entry;
        entry.obligation_id = o.id;
        entry.prove_ref = ob.prove_ref;
        entry.source = "mrs_proof_audit";

        std::string evidence;
        std::string fail;
        const bool ok = evaluate_obligation_witness(o.id, ob, bundle, ctx, &evidence, &fail);
        entry.ok = ok;
        entry.witness = evidence;
        entry.failure_reason = fail;
        rep.entries.push_back(entry);

        if (!ok) {
            rep.ok = false;
            apply_numeric_evidence(graph, o.id, false, evidence, fail);
            continue;
        }

        if (o.proof_class == ProofClass::Composition && ob.prove_kind != MrsProofBodyKind::Infer &&
            ob.prove_ref.empty())
            continue;

        apply_numeric_evidence(graph, o.id, true, evidence.empty() ? "mrs_witness:" + o.id : evidence);
    }

    return rep;
}

bool export_mrs_proof_audit_json(const std::string& path, const MrsProofAuditReport& report) {
    std::ofstream out(path);
    if (!out) return false;
    out << "{\n  \"ok\": " << (report.ok ? "true" : "false") << ",\n  \"entries\": [\n";
    for (size_t i = 0; i < report.entries.size(); ++i) {
        const auto& e = report.entries[i];
        out << "    {\"obligation_id\": \"" << e.obligation_id << "\", "
            << "\"source\": \"" << e.source << "\", "
            << "\"witness\": \"" << e.witness << "\", "
            << "\"prove_ref\": \"" << e.prove_ref << "\", "
            << "\"ok\": " << (e.ok ? "true" : "false") << ", "
            << "\"failure_reason\": \"" << e.failure_reason << "\"}";
        if (i + 1 < report.entries.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::AnaVM
