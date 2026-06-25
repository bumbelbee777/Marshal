#include "MrsLadderProofGate.hxx"

#include "MrsForallExtension.hxx"
#include "MrsInductionExtension.hxx"
#include "MrsConvergenceExtension.hxx"
#include "MrsMath.hxx"
#include "MrsProofLogic.hxx"
#include "MrsProveSpine.hxx"
#include "MrsTransform.hxx"

#include "MrsProofAudit.hxx"

namespace Marshal::AnaVM {

bool ladder_bsd_bounds_ok(const Heat::GLn::GL2BSDProofReport& report) {
    return report.l_grid_ok && report.holomorphy_ok && report.sha_gap_ok;
}

bool ladder_hodge_bounds_ok(const Heat::GLn::GL3HodgeProofReport& report) {
    return report.rank3_contract_ok && report.cycle_constructive_ok;
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
        id == "goldbach_rh_prerequisite" || id == "ym_hodge_prerequisite") {
        if (id == "ym_hodge_prerequisite") {
            if (!ctx.hodge_capstone_ok) {
                if (fail) *fail = "Hodge capstone not closed";
                return false;
            }
            if (evidence) *evidence = "hodge_capstone_ok";
            return true;
        }
        if (!ctx.rh_capstone_ok) {
            if (fail) *fail = "RH capstone not closed";
            return false;
        }
        if (evidence) *evidence = "rh_capstone_ok";
        return true;
    }

    if (!ob.witness_expr.empty()) {
        std::string hard_err;
        if (!witness_expr_passes_hardening("MarshalLadder", id, ob.witness_expr, &hard_err)) {
            if (fail) *fail = "witness hardening: " + hard_err;
            return false;
        }
        if (ob.proof_class == ProofClass::Universal) {
            const std::string prove_ref =
                !ob.prove_ref.empty() ? ob.prove_ref : ob.extend_via;
            if (!prove_ref.empty() && !bundle_has_prove_ref(bundle, prove_ref)) {
                if (fail) *fail = "missing prove ref " + prove_ref;
                return false;
            }
            if (!ctx.cfg) {
                if (fail) *fail = "missing config for witness_expr";
                return false;
            }
            MrsMathWitnessEnv env = build_mrs_math_witness_env(ctx, *ctx.cfg);
        merge_mrs_arrays_into_env(bundle, &env);
        merge_mrs_fns_into_env(bundle, &env);
            std::string detail;
            std::string err;
            const bool ok =
                evaluate_forall_extension_witness(ob, bundle, env, &detail, &err);
            if (!ok) {
                if (fail) *fail = err.empty() ? "forall_extension witness false" : err;
                return false;
            }
            if (evidence) *evidence = detail;
            return true;
        }
        if (ob.proof_class == ProofClass::Inductive) {
            const std::string prove_ref =
                !ob.prove_ref.empty() ? ob.prove_ref : ob.extend_via;
            if (!prove_ref.empty() && !bundle_has_prove_ref(bundle, prove_ref)) {
                if (fail) *fail = "missing prove ref " + prove_ref;
                return false;
            }
            if (!ctx.cfg) {
                if (fail) *fail = "missing config for witness_expr";
                return false;
            }
            MrsMathWitnessEnv env = build_mrs_math_witness_env(ctx, *ctx.cfg);
        merge_mrs_arrays_into_env(bundle, &env);
        merge_mrs_fns_into_env(bundle, &env);
            std::string detail;
            std::string err;
            const bool ok = evaluate_induction_witness(ob, bundle, env, &detail, &err);
            if (!ok) {
                if (fail) *fail = err.empty() ? "induction witness false" : err;
                return false;
            }
            if (evidence) *evidence = detail;
            return true;
        }
        if (ob.proof_class == ProofClass::Convergent) {
            const std::string prove_ref =
                !ob.prove_ref.empty() ? ob.prove_ref : ob.extend_via;
            if (!prove_ref.empty() && !bundle_has_prove_ref(bundle, prove_ref)) {
                if (fail) *fail = "missing prove ref " + prove_ref;
                return false;
            }
            if (!ctx.cfg) {
                if (fail) *fail = "missing config for witness_expr";
                return false;
            }
            MrsMathWitnessEnv env = build_mrs_math_witness_env(ctx, *ctx.cfg);
            merge_mrs_arrays_into_env(bundle, &env);
        merge_mrs_fns_into_env(bundle, &env);
            std::string detail;
            std::string err;
            const bool ok = evaluate_convergence_witness(ob, bundle, env, &detail, &err);
            if (!ok) {
                if (fail) *fail = err.empty() ? "convergence witness false" : err;
                return false;
            }
            if (evidence) *evidence = detail;
            return true;
        }
        if (ob.proof_class == ProofClass::Rewrite) {
            if (!ctx.cfg) {
                if (fail) *fail = "missing config for rewrite witness";
                return false;
            }
            MrsMathWitnessEnv env = build_mrs_math_witness_env(ctx, *ctx.cfg);
            merge_mrs_arrays_into_env(bundle, &env);
            merge_mrs_fns_into_env(bundle, &env);
            std::string detail;
            std::string err;
            const bool ok = evaluate_rewrite_obligation(ob, bundle, env, &detail, &err);
            if (!ok) {
                if (fail) *fail = err.empty() ? "rewrite witness false" : err;
                return false;
            }
            if (evidence) *evidence = detail;
            return true;
        }
        if (ob.proof_class == ProofClass::DecisionProcedure) {
            if (!ctx.cfg) {
                if (fail) *fail = "missing config for decision_procedure witness";
                return false;
            }
            MrsMathWitnessEnv env = build_mrs_math_witness_env(ctx, *ctx.cfg);
            merge_mrs_arrays_into_env(bundle, &env);
            merge_mrs_fns_into_env(bundle, &env);
            std::string detail;
            std::string err;
            const bool ok = evaluate_decision_procedure_obligation(ob, env, &detail, &err);
            if (!ok) {
                if (fail) *fail = err.empty() ? "decision_procedure witness false" : err;
                return false;
            }
            if (evidence) *evidence = detail;
            return true;
        }
        if (ob.proof_class == ProofClass::Composition || ob.proof_class == ProofClass::Analytic ||
            ob.proof_class == ProofClass::ClassicalImport ||
            ob.proof_class == ProofClass::Reduction ||
            ob.proof_class == ProofClass::AnalyticOpen) {
            const std::string prove_ref =
                !ob.prove_ref.empty() ? ob.prove_ref : ob.extend_via;
            if (!prove_ref.empty() && !bundle_has_prove_ref(bundle, prove_ref)) {
                if (fail) *fail = "missing prove ref " + prove_ref;
                return false;
            }
        } else if (ob.prove_kind != MrsProofBodyKind::Infer && !ob.prove_ref.empty() &&
                   !bundle_has_prove_ref(bundle, ob.prove_ref)) {
            if (fail) *fail = "missing prove ref " + ob.prove_ref;
            return false;
        }
        if (!ctx.cfg) {
            if (fail) *fail = "missing config for witness_expr";
            return false;
        }
        MrsMathWitnessEnv env = build_mrs_math_witness_env(ctx, *ctx.cfg);
        merge_mrs_arrays_into_env(bundle, &env);
        merge_mrs_fns_into_env(bundle, &env);
        const auto defs = collect_mrs_defs(bundle);
        std::string detail;
        std::string err;
        const std::string expanded_witness = expand_mrs_defs(ob.witness_expr, defs);
        const bool ok = evaluate_mrs_witness_expr(expanded_witness, env, &detail, &err);
        if (!ok) {
            if (fail) *fail = err.empty() ? "witness_expr false: " + ob.witness_expr : err;
            return false;
        }
        std::string combined = detail.empty() ? "mrs_witness_expr" : "mrs_math:" + detail;
        const std::string prove_ref = !ob.prove_ref.empty() ? ob.prove_ref : ob.extend_via;
        if (!prove_ref.empty() && bundle_has_prove_ref(bundle, prove_ref)) {
            const MrsProveDecl* prove = nullptr;
            for (const auto& m : bundle.merged_modules) {
                for (const auto& p : m.proves) {
                    if (p.name == prove_ref) {
                        prove = &p;
                        break;
                    }
                }
                if (prove) break;
            }
            if (prove && mrs_prove_body_is_formula_script(prove->body)) {
                const MrsProveScript script = parse_mrs_prove_script(prove->body);
                std::string script_detail;
                std::string script_err;
                if (!evaluate_mrs_prove_witnesses(script, bundle, env, &script_detail, &script_err)) {
                    if (fail) *fail = script_err;
                    return false;
                }
                combined += ";mrs_prove:" + script_detail;
            }
        }
        if (evidence) *evidence = combined;
        return true;
    }

    if (ob.proof_class == ProofClass::Composition && ob.prove_ref.empty()) return true;
    if (fail) *fail = "no ladder witness for " + id + " (add witness_expr in MRS)";
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
            ob.prove_kind != MrsProofBodyKind::Infer && ob.prove_ref.empty() &&
            ob.witness_expr.empty())
            continue;

        MrsProofAuditEntry entry;
        entry.obligation_id = o.id;
        entry.prove_ref = ob.prove_ref;
        entry.source = "mrs_ladder_audit";
        entry.tier = mrs_obligation_tier(ob.proof_class);
        entry.referee_class =
            mrs_obligation_referee_class(o.id, ob.proof_class, ob.witness_expr);

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
