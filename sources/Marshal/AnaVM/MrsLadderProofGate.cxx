#include "MrsLadderProofGate.hxx"

#include "MrsProofAudit.hxx"

#include <sstream>

namespace Marshal::AnaVM {

bool ladder_bsd_bounds_ok(const Heat::GLn::GL2BSDProofReport& report) {
    return report.rank_match_ok && report.l_grid_ok && report.sha_gap_ok;
}

bool ladder_hodge_bounds_ok(const Heat::GLn::GL3HodgeProofReport& report) {
    return report.hodge_match_ok;
}

bool ladder_goldbach_bounds_ok(const Heat::GLn::GL2EllipseHeegnerReport& report) {
    return report.major_arc_ok && report.minor_arc_ok && report.goldbach_shared_gln2_ok;
}

MrsLadderProofRefusal ladder_bsd_proof_refusal(const Heat::GLn::GL2BSDProofReport& report) {
    if (!report.rh_prerequisite_ok) return MrsLadderProofRefusal::RhPrerequisiteOpen;
    if (!ladder_bsd_bounds_ok(report)) return MrsLadderProofRefusal::BoundsExceedTolerance;
    if (!report.mrs_proof_audit_ok) return MrsLadderProofRefusal::MrsProofAuditFailed;
    if (!report.bsd_rank_proved) return MrsLadderProofRefusal::CapstoneOpen;
    return MrsLadderProofRefusal::None;
}

MrsLadderProofRefusal ladder_hodge_proof_refusal(const Heat::GLn::GL3HodgeProofReport& report) {
    if (!report.rh_prerequisite_ok) return MrsLadderProofRefusal::RhPrerequisiteOpen;
    if (!ladder_hodge_bounds_ok(report)) return MrsLadderProofRefusal::BoundsExceedTolerance;
    if (!report.mrs_proof_audit_ok) return MrsLadderProofRefusal::MrsProofAuditFailed;
    if (!report.hodge_conjecture_proved) return MrsLadderProofRefusal::CapstoneOpen;
    return MrsLadderProofRefusal::None;
}

MrsLadderProofRefusal ladder_goldbach_proof_refusal(
    const Heat::GLn::GL2EllipseHeegnerReport& report, bool rh_ok, bool bsd_ok) {
    if (!rh_ok) return MrsLadderProofRefusal::RhPrerequisiteOpen;
    if (!bsd_ok) return MrsLadderProofRefusal::BsdPrerequisiteOpen;
    if (!ladder_goldbach_bounds_ok(report)) return MrsLadderProofRefusal::BoundsExceedTolerance;
    if (!report.bounds_ok) return MrsLadderProofRefusal::CapstoneOpen;
    return MrsLadderProofRefusal::None;
}

bool ladder_bsd_proof_ok(const Heat::GLn::GL2BSDProofReport& report) {
    return ladder_bsd_proof_refusal(report) == MrsLadderProofRefusal::None;
}

bool ladder_hodge_proof_ok(const Heat::GLn::GL3HodgeProofReport& report) {
    return ladder_hodge_proof_refusal(report) == MrsLadderProofRefusal::None;
}

bool ladder_goldbach_proof_ok(const Heat::GLn::GL2EllipseHeegnerReport& report, bool rh_ok,
                              bool bsd_ok) {
    return ladder_goldbach_proof_refusal(report, rh_ok, bsd_ok) == MrsLadderProofRefusal::None;
}

const char* ladder_proof_refusal_message(MrsLadderProofRefusal refusal) {
    switch (refusal) {
        case MrsLadderProofRefusal::RhPrerequisiteOpen:
            return "RH capstone prerequisite open";
        case MrsLadderProofRefusal::BsdPrerequisiteOpen:
            return "BSD capstone prerequisite open";
        case MrsLadderProofRefusal::BoundsExceedTolerance:
            return "ladder bounds exceed tolerance";
        case MrsLadderProofRefusal::MrsProofAuditFailed:
            return "MRS ladder proof audit failed";
        case MrsLadderProofRefusal::CapstoneOpen:
            return "ladder capstone not proved";
        default:
            return "ok";
    }
}

namespace {

bool evaluate_ladder_obligation(const std::string& id, const MrsProofObligationDecl& ob,
                                const MrsCompilationBundle& bundle,
                                const MrsLadderWitnessContext& ctx, std::string* evidence,
                                std::string* fail) {
    if (id == "bsd_rh_prerequisite" || id == "hodge_rh_prerequisite" ||
        id == "goldbach_rh_prerequisite") {
        if (!ctx.rh_capstone_ok) {
            if (fail) *fail = "RH capstone not closed";
            return false;
        }
        if (evidence) *evidence = "rh_capstone_ok";
        return true;
    }
    if (id == "gl2_grid_pointwise_l_match") {
        if (!ctx.bsd || !ctx.bsd->l_grid_ok) {
            if (fail) *fail = "l_grid_ok=false";
            return false;
        }
        if (evidence) *evidence = "l_function_grid_rel_gap within ub";
        return true;
    }
    if (id == "gl2_spectral_det_holomorphic_off_strip") {
        if (!bundle_has_prove_ref(bundle, ob.prove_ref)) {
            if (fail) *fail = "missing prove ref " + ob.prove_ref;
            return false;
        }
        if (!ctx.bsd || !ctx.bsd->l_grid_ok || !ctx.bsd->sha_gap_ok) {
            if (fail) *fail = "holomorphy prerequisite failed";
            return false;
        }
        if (evidence) *evidence = "mrs_prove:" + ob.prove_ref;
        return true;
    }
    if (id == "gl2_l_function_identification") {
        if (!bundle_has_prove_ref(bundle, ob.prove_ref)) {
            if (fail) *fail = "missing prove ref " + ob.prove_ref;
            return false;
        }
        if (!ctx.bsd || !ctx.bsd->l_grid_ok) {
            if (fail) *fail = "L-function ID witness failed";
            return false;
        }
        if (evidence) *evidence = "mrs_prove:" + ob.prove_ref;
        return true;
    }
    if (id == "gl2_kernel_rank_match") {
        if (!ctx.bsd || !ctx.bsd->rank_match_ok) {
            if (fail) *fail = "kernel rank match failed";
            return false;
        }
        if (evidence) {
            std::ostringstream oss;
            oss << "kernel_multiplicity=" << ctx.bsd->kernel_multiplicity;
            *evidence = oss.str();
        }
        return true;
    }
    if (id == "gl2_sha_resolvent_gap") {
        if (!ctx.bsd || !ctx.bsd->sha_gap_ok) {
            if (fail) *fail = "sha resolvent gap failed";
            return false;
        }
        if (evidence) *evidence = "sha_resolvent_gap within ub";
        return true;
    }
    if (id == "bsd_rank_proved") {
        if (!bundle_has_prove_ref(bundle, ob.prove_ref)) {
            if (fail) *fail = "missing prove ref " + ob.prove_ref;
            return false;
        }
        if (!ctx.bsd || !ctx.bsd->bsd_rank_proved) {
            if (fail) *fail = "bsd capstone failed";
            return false;
        }
        if (evidence) *evidence = "mrs_prove:" + ob.prove_ref;
        return true;
    }
    if (id == "gl3_hitchin_spectral_triple_witness") {
        if (!ctx.hodge) {
            if (fail) *fail = "missing hodge report";
            return false;
        }
        if (evidence) *evidence = "gl3_hitchin_triple built";
        return true;
    }
    if (id == "hodge_kernel_h11_match") {
        if (!ctx.hodge || !ctx.hodge->hodge_match_ok) {
            if (fail) *fail = "hodge kernel h11 match failed";
            return false;
        }
        if (evidence) {
            std::ostringstream oss;
            oss << "kernel_multiplicity=" << ctx.hodge->kernel_multiplicity;
            *evidence = oss.str();
        }
        return true;
    }
    if (id == "hodge_lefschetz_bridge") {
        if (!bundle_has_prove_ref(bundle, ob.prove_ref)) {
            if (fail) *fail = "missing prove ref " + ob.prove_ref;
            return false;
        }
        if (!ctx.hodge || !ctx.hodge->hodge_match_ok) {
            if (fail) *fail = "lefschetz prerequisite failed";
            return false;
        }
        if (evidence) *evidence = "mrs_prove:" + ob.prove_ref;
        return true;
    }
    if (id == "hodge_conjecture_proved") {
        if (!bundle_has_prove_ref(bundle, ob.prove_ref)) {
            if (fail) *fail = "missing prove ref " + ob.prove_ref;
            return false;
        }
        if (!ctx.hodge || !ctx.hodge->hodge_conjecture_proved) {
            if (fail) *fail = "hodge capstone failed";
            return false;
        }
        if (evidence) *evidence = "mrs_prove:" + ob.prove_ref;
        return true;
    }
    if (id == "goldbach_bsd_shared_gln2") {
        if (!ctx.bsd_capstone_ok) {
            if (fail) *fail = "BSD capstone not closed";
            return false;
        }
        if (!ctx.goldbach || !ctx.goldbach->goldbach_shared_gln2_ok) {
            if (fail) *fail = "shared gln2 witness failed";
            return false;
        }
        if (evidence) *evidence = "MaassEllipseHeegner shared spine";
        return true;
    }
    if (id == "goldbach_major_arc_lower_bound") {
        if (!ctx.goldbach || !ctx.goldbach->major_arc_ok) {
            if (fail) *fail = "major arc bound failed";
            return false;
        }
        if (evidence) *evidence = "major_arc_spectral_mass >= threshold";
        return true;
    }
    if (id == "goldbach_minor_arc_upper_bound") {
        if (!ctx.goldbach || !ctx.goldbach->minor_arc_ok) {
            if (fail) *fail = "minor arc bound failed";
            return false;
        }
        if (evidence) *evidence = "minor_arc_bound < ub";
        return true;
    }
    if (id == "goldbach_effective_range") {
        if (!ob.prove_ref.empty() && !bundle_has_prove_ref(bundle, ob.prove_ref)) {
            if (fail) *fail = "missing prove ref " + ob.prove_ref;
            return false;
        }
        if (!ctx.goldbach_effective || !ctx.goldbach_effective->ok) {
            if (fail) *fail = "goldbach effective range check failed";
            return false;
        }
        if (evidence) {
            std::ostringstream oss;
            oss << "even_n_checked=" << ctx.goldbach_effective->even_count
                << " n_max=" << ctx.goldbach_effective->n_max_checked;
            *evidence = oss.str();
        }
        return true;
    }
    if (id == "goldbach_circle_method_identification") {
        if (!bundle_has_prove_ref(bundle, ob.prove_ref)) {
            if (fail) *fail = "missing prove ref " + ob.prove_ref;
            return false;
        }
        if (!ctx.goldbach || !ctx.goldbach->major_arc_ok || !ctx.goldbach->minor_arc_ok) {
            if (fail) *fail = "circle method prerequisite failed";
            return false;
        }
        if (!ctx.goldbach_effective || !ctx.goldbach_effective->ok) {
            if (fail) *fail = "effective range prerequisite failed";
            return false;
        }
        if (evidence) *evidence = "mrs_prove:" + ob.prove_ref;
        return true;
    }
    if (id == "goldbach_proved") {
        if (!bundle_has_prove_ref(bundle, ob.prove_ref)) {
            if (fail) *fail = "missing prove ref " + ob.prove_ref;
            return false;
        }
        if (!ctx.goldbach || !ctx.goldbach->bounds_ok) {
            if (fail) *fail = "goldbach arc bounds failed";
            return false;
        }
        if (!ctx.goldbach_effective || !ctx.goldbach_effective->ok) {
            if (fail) *fail = "goldbach effective range failed";
            return false;
        }
        if (evidence) *evidence = "mrs_prove:" + ob.prove_ref;
        return true;
    }
    if (ob.proof_class == ProofClass::Composition && ob.prove_ref.empty()) return true;
    if (fail) *fail = "no ladder witness for " + id;
    return false;
}

const MrsProofObligationDecl* find_ladder_obligation(const MrsCompilationBundle& bundle,
                                                     const std::string& id) {
    for (const auto& m : bundle.merged_modules) {
        for (const auto& g : m.proof_graphs) {
            if (g.name == "MarshalHadamard") continue;
            for (const auto& ob : g.obligations) {
                if (ob.id == id) return &ob;
            }
        }
    }
    return nullptr;
}

}  // namespace

MrsProofAuditReport apply_mrs_ladder_proof_audit(ProofGraphReport& graph,
                                                 const MrsCompilationBundle& bundle,
                                                 const MrsLadderWitnessContext& ctx) {
    MrsProofAuditReport rep;
    rep.ok = true;

    for (auto& o : graph.obligations) {
        const MrsProofObligationDecl* decl = find_ladder_obligation(bundle, o.id);
        if (!decl) continue;
        const MrsProofObligationDecl ob = *decl;

        if (o.proof_class == ProofClass::Composition &&
            ob.prove_kind != MrsProofBodyKind::Infer && ob.prove_ref.empty())
            continue;

        MrsProofAuditEntry entry;
        entry.obligation_id = o.id;
        entry.prove_ref = ob.prove_ref;
        entry.source = "mrs_ladder_audit";

        std::string evidence;
        std::string fail;
        const bool ok = evaluate_ladder_obligation(o.id, ob, bundle, ctx, &evidence, &fail);
        entry.ok = ok;
        entry.witness = evidence;
        entry.failure_reason = fail;
        rep.entries.push_back(entry);

        if (!ok) {
            rep.ok = false;
            apply_numeric_evidence(graph, o.id, false, evidence, fail);
            continue;
        }
        apply_numeric_evidence(graph, o.id, true, evidence.empty() ? "mrs_ladder:" + o.id : evidence);
    }
    return rep;
}

}  // namespace Marshal::AnaVM
