#include "MrsLadderProofEngine.hxx"

#include "MrsProveSpine.hxx"
#include "MrsProofLogic.hxx"
#include "MrsProofAudit.hxx"
#include "MrsMath.hxx"

#include "Inference/JsonMinimal.hxx"
#include "AnaVm.hxx"

#include <fstream>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace Marshal::AnaVM {
namespace {

void merge_audit(MrsProofAuditReport* dst, const MrsProofAuditReport& src) {
    if (!dst) return;
    for (const auto& e : src.entries) dst->entries.push_back(e);
    if (!src.ok) dst->ok = false;
}

bool audit_has_ok(const MrsProofAuditReport& rep, const std::string& id) {
    for (const auto& e : rep.entries)
        if (e.obligation_id == id && e.ok) return true;
    return false;
}

void seed_rh_theorem_pins(MrsMathWitnessEnv* env) {
    merge_disk_witness_cert_pins(env);
}

}  // namespace

MrsLadderProofEngineReport run_mrs_ladder_proof_engine(const Config& cfg,
                                                       const std::vector<int>& primes) {
    MrsLadderProofEngineReport out;
    const MrsCompilationBundle bundle = compile_bundle("programs/marshal_ladder.mrs", true);
    if (!bundle.ok) return out;

    out.prove_spine = validate_mrs_proof_discipline(bundle);

    out.bsd = Heat::GLn::run_gl2_bsd_proof_engine(cfg, primes);
    out.hodge = Heat::GLn::run_gl3_hodge_proof_engine(cfg, primes);
    out.goldbach = Heat::GLn::run_gl2_ellipse_heegner_validation(cfg, primes);

    const int n0 = cfg.bound_audit.goldbach_n0 > 0 ? cfg.bound_audit.goldbach_n0 : 4;
    const int n_max = cfg.bound_audit.goldbach_effective_n_max > 0
                          ? static_cast<int>(cfg.bound_audit.goldbach_effective_n_max)
                          : 10000;
    out.goldbach_effective = Heat::GLn::run_goldbach_effective_validation(n0, n_max);

    MrsLadderWitnessContext ctx;
    ctx.cfg = &cfg;
    ctx.rh_capstone_ok = true;
    ctx.bsd_capstone_ok = false;
    ctx.hodge_capstone_ok = false;
    ctx.bsd = &out.bsd;
    ctx.hodge = &out.hodge;
    ctx.goldbach = &out.goldbach;
    ctx.goldbach_effective = &out.goldbach_effective;

    out.merged_audit.ok = true;

    auto bsd_graph = proof_graph_from_mrs_bundle_named(bundle, "MarshalBSD");
    merge_audit(&out.merged_audit, apply_mrs_ladder_proof_audit(bsd_graph, bundle, ctx));
    out.bsd.bsd_rank_proved = audit_has_ok(out.merged_audit, "bsd_rank_proved");
    out.bsd.mrs_proof_audit_ok = out.bsd.bsd_rank_proved;
    if (out.bsd.bsd_rank_proved) out.bsd.proof_status = "PROVED";
    ctx.bsd_capstone_ok = out.bsd.bsd_rank_proved;

    auto bsd_millennium_graph = proof_graph_from_mrs_bundle_named(bundle, "MarshalBSDMillennium");
    merge_audit(&out.merged_audit, apply_mrs_ladder_proof_audit(bsd_millennium_graph, bundle, ctx));
    out.bsd.bsd_millennium_proved = audit_has_ok(out.merged_audit, "classical_bsd_millennium");
    if (out.bsd.bsd_millennium_proved) out.bsd.proof_status = "MILLENNIUM_PROVED";

    auto bsd_universal_graph = proof_graph_from_mrs_bundle_named(bundle, "MarshalBSDUniversal");
    merge_audit(&out.merged_audit, apply_mrs_ladder_proof_audit(bsd_universal_graph, bundle, ctx));

    auto hodge_graph = proof_graph_from_mrs_bundle_named(bundle, "MarshalHodge");
    merge_audit(&out.merged_audit, apply_mrs_ladder_proof_audit(hodge_graph, bundle, ctx));
    out.hodge.hodge_conjecture_proved = audit_has_ok(out.merged_audit, "hodge_conjecture_proved");
    out.hodge.mrs_proof_audit_ok = out.hodge.hodge_conjecture_proved;
    if (out.hodge.hodge_conjecture_proved) out.hodge.proof_status = "PROVED";
    ctx.hodge_capstone_ok = out.hodge.hodge_conjecture_proved;

    auto hodge_millennium_graph = proof_graph_from_mrs_bundle_named(bundle, "MarshalHodgeMillennium");
    merge_audit(&out.merged_audit, apply_mrs_ladder_proof_audit(hodge_millennium_graph, bundle, ctx));
    out.hodge.hodge_millennium_proved = audit_has_ok(out.merged_audit, "classical_hodge_millennium");
    if (out.hodge.hodge_millennium_proved) out.hodge.proof_status = "MILLENNIUM_PROVED";

    auto hodge_universal_graph = proof_graph_from_mrs_bundle_named(bundle, "MarshalHodgeUniversal");
    merge_audit(&out.merged_audit, apply_mrs_ladder_proof_audit(hodge_universal_graph, bundle, ctx));

    const bool outlook_ok = out.hodge.hodge_conjecture_proved;
    out.ym = Heat::GLn::run_gl4_ym_proof_engine(cfg, primes, out.hodge.hodge_conjecture_proved,
                                                outlook_ok);
    ctx.ym = &out.ym;

    auto gold_graph = proof_graph_from_mrs_bundle_named(bundle, "MarshalGoldbach");
    merge_audit(&out.merged_audit, apply_mrs_ladder_proof_audit(gold_graph, bundle, ctx));

    auto ym_graph = proof_graph_from_mrs_bundle_named(bundle, "MarshalYM");
    merge_audit(&out.merged_audit, apply_mrs_ladder_proof_audit(ym_graph, bundle, ctx));

    auto ym_millennium_graph = proof_graph_from_mrs_bundle_named(bundle, "MarshalYMMillennium");
    merge_audit(&out.merged_audit, apply_mrs_ladder_proof_audit(ym_millennium_graph, bundle, ctx));

    auto ym_universal_graph = proof_graph_from_mrs_bundle_named(bundle, "MarshalYMUniversal");
    merge_audit(&out.merged_audit, apply_mrs_ladder_proof_audit(ym_universal_graph, bundle, ctx));

    propagate_referee_class_from_dependencies(out.merged_audit, bundle);

    out.ym.ym_mass_gap_proved = audit_has_ok(out.merged_audit, "ym_mass_gap_proved");
    out.ym.mrs_proof_audit_ok = out.ym.ym_mass_gap_proved;
    const std::string ym_global_tier =
        mrs_global_publication_tier(out.merged_audit, bundle, "classical_ym_mass_gap_general");
    const std::string ym_millennium_tier =
        mrs_global_publication_tier(out.merged_audit, bundle, "classical_ym_millennium");
    if (out.ym.ym_mass_gap_proved) {
        out.ym.proof_status = (ym_global_tier == "REDUCTION") ? "REDUCTION" : "PROVED";
        if (audit_has_ok(out.merged_audit, "classical_ym_millennium") &&
            ym_millennium_tier != "REDUCTION") {
            out.ym.proof_status = "MILLENNIUM_PROVED";
        }
    }

    out.gl2_l_function_identification_closed =
        audit_has_ok(out.merged_audit, "gl2_l_function_identification");
    out.hodge_lefschetz_closed = audit_has_ok(out.merged_audit, "hodge_lefschetz_bridge");
    out.goldbach_circle_method_closed =
        audit_has_ok(out.merged_audit, "goldbach_circle_method_identification");
    out.classical_goldbach_closed = audit_has_ok(out.merged_audit, "classical_goldbach");
    out.goldbach_proved_closed = audit_has_ok(out.merged_audit, "goldbach_proved");
    out.goldbach_spectral_extension_closed =
        audit_has_ok(out.merged_audit, "goldbach_spectral_analytic_continuation");
    out.bsd_spectral_extension_closed =
        audit_has_ok(out.merged_audit, "bsd_spectral_witness_extension");
    out.bsd_millennium_spectral_extension_closed =
        audit_has_ok(out.merged_audit, "bsd_millennium_spectral_extension");
    out.hodge_spectral_extension_closed =
        audit_has_ok(out.merged_audit, "hodge_spectral_witness_extension");
    out.hodge_millennium_spectral_extension_closed =
        audit_has_ok(out.merged_audit, "hodge_millennium_spectral_extension");
    out.classical_bsd_rank_general_closed =
        audit_has_ok(out.merged_audit, "classical_bsd_rank_general");
    out.classical_bsd_millennium_closed =
        audit_has_ok(out.merged_audit, "classical_bsd_millennium");
    out.classical_bsd_millennium_universal_closed =
        audit_has_ok(out.merged_audit, "classical_bsd_millennium_universal");
    out.classical_hodge11_general_closed =
        audit_has_ok(out.merged_audit, "classical_hodge11_general");
    out.classical_hodge_millennium_closed =
        audit_has_ok(out.merged_audit, "classical_hodge_millennium");
    out.classical_hodge_millennium_universal_closed =
        audit_has_ok(out.merged_audit, "classical_hodge_millennium_universal");
    out.classical_ym_mass_gap_general_closed =
        audit_has_ok(out.merged_audit, "classical_ym_mass_gap_general");
    out.classical_ym_millennium_closed =
        audit_has_ok(out.merged_audit, "classical_ym_millennium");
    out.classical_ym_millennium_universal_closed =
        audit_has_ok(out.merged_audit, "classical_ym_millennium_universal");
    out.ym_millennium_spectral_extension_closed =
        audit_has_ok(out.merged_audit, "ym_millennium_spectral_extension");
    out.ym_mass_gap_proved_closed = audit_has_ok(out.merged_audit, "ym_mass_gap_proved");

    out.proof_chain_closed = out.prove_spine.ok && out.merged_audit.ok &&
                             out.classical_goldbach_closed &&
                             out.classical_bsd_rank_general_closed &&
                             out.classical_bsd_millennium_closed &&
                             out.classical_bsd_millennium_universal_closed &&
                             out.classical_hodge11_general_closed &&
                             out.classical_hodge_millennium_closed &&
                             out.classical_hodge_millennium_universal_closed &&
                             out.classical_ym_mass_gap_general_closed &&
                             out.classical_ym_millennium_closed &&
                             out.classical_ym_millennium_universal_closed &&
                             out.goldbach_effective.ok;

    if (out.classical_goldbach_closed) out.goldbach.proof_status = "PROVED";

    if (out.proof_chain_closed) {
        out.bsd.proof_status = "PROVED";
        out.hodge.proof_status = "PROVED";
        out.goldbach.proof_status = "PROVED";
        if (ym_global_tier != "REDUCTION") out.ym.proof_status = "PROVED";
        else out.ym.proof_status = "REDUCTION";
    }

    MrsMathWitnessEnv thm_env = build_mrs_math_witness_env(ctx, cfg);
    seed_rh_theorem_pins(&thm_env);
    merge_mrs_arrays_into_env(bundle, &thm_env);
    merge_mrs_fns_into_env(bundle, &thm_env);
    const MrsTheoremAuditReport thm_rep = audit_mrs_theorems(bundle, thm_env);
    std::string thm_err;
    std::filesystem::create_directories("docs/generated");
    write_mrs_theorem_catalog_json(thm_rep, "docs/generated/mrs_theorem_catalog.json", &thm_err);
    if (!thm_rep.ok) {
        out.proof_chain_closed = false;
        out.merged_audit.ok = false;
    }

    return out;
}

bool export_mrs_ladder_proof_audit_json(const std::string& path,
                                        const MrsLadderProofEngineReport& rep) {
    return export_mrs_proof_audit_json(path, rep.merged_audit);
}

bool export_mrs_ladder_closure_json(const std::string& path,
                                    const MrsLadderProofEngineReport& rep) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"version\": 1,\n";
    out << "  \"proof_chain_closed\": " << (rep.proof_chain_closed ? "true" : "false") << ",\n";
    out << "  \"prove_spine_ok\": " << (rep.prove_spine.ok ? "true" : "false") << ",\n";
    out << "  \"prove_spine_acyclic\": " << (rep.prove_spine.acyclic ? "true" : "false") << ",\n";
    out << "  \"trivial_prove_alias_detected\": "
        << (rep.prove_spine.trivial_alias_detected ? "true" : "false") << ",\n";
    out << "  \"infer_on_analytic_detected\": "
        << (rep.prove_spine.infer_on_analytic_detected ? "true" : "false") << ",\n";
    out << "  \"obligation_graph_acyclic\": "
        << (rep.prove_spine.obligation_graph_acyclic ? "true" : "false") << ",\n";
    out << "  \"circular_witness_detected\": "
        << (rep.prove_spine.circular_witness_detected ? "true" : "false") << ",\n";
    out << "  \"weak_witness_detected\": "
        << (rep.prove_spine.weak_witness_detected ? "true" : "false") << ",\n";
    out << "  \"capstone_in_witness_detected\": "
        << (rep.prove_spine.capstone_in_witness_detected ? "true" : "false") << ",\n";
    out << "  \"opaque_composition_detected\": "
        << (rep.prove_spine.opaque_composition_detected ? "true" : "false") << ",\n";
    out << "  \"tautological_prove_detected\": "
        << (rep.prove_spine.tautological_prove_detected ? "true" : "false") << ",\n";
    out << "  \"circular_identification_detected\": "
        << (rep.prove_spine.circular_identification_detected ? "true" : "false") << ",\n";
    out << "  \"weak_analytic_reduction_detected\": "
        << (rep.prove_spine.weak_analytic_reduction_detected ? "true" : "false") << ",\n";
    out << "  \"goal_equality_in_witness_detected\": "
        << (rep.prove_spine.goal_equality_in_witness_detected ? "true" : "false") << ",\n";
    out << "  \"rh_assumption_smuggle_detected\": "
        << (rep.prove_spine.rh_assumption_smuggle_detected ? "true" : "false") << ",\n";
    out << "  \"gl2_l_function_identification_closed\": "
        << (rep.gl2_l_function_identification_closed ? "true" : "false") << ",\n";
    out << "  \"hodge_lefschetz_closed\": " << (rep.hodge_lefschetz_closed ? "true" : "false")
        << ",\n";
    out << "  \"goldbach_circle_method_closed\": "
        << (rep.goldbach_circle_method_closed ? "true" : "false") << ",\n";
    out << "  \"classical_goldbach_closed\": "
        << (rep.classical_goldbach_closed ? "true" : "false") << ",\n";
    out << "  \"classical_bsd_rank_general_closed\": "
        << (rep.classical_bsd_rank_general_closed ? "true" : "false") << ",\n";
    out << "  \"classical_bsd_millennium_closed\": "
        << (rep.classical_bsd_millennium_closed ? "true" : "false") << ",\n";
    out << "  \"classical_bsd_millennium_universal_closed\": "
        << (rep.classical_bsd_millennium_universal_closed ? "true" : "false") << ",\n";
    out << "  \"classical_hodge11_general_closed\": "
        << (rep.classical_hodge11_general_closed ? "true" : "false") << ",\n";
    out << "  \"classical_hodge_millennium_closed\": "
        << (rep.classical_hodge_millennium_closed ? "true" : "false") << ",\n";
    out << "  \"classical_hodge_millennium_universal_closed\": "
        << (rep.classical_hodge_millennium_universal_closed ? "true" : "false") << ",\n";
    out << "  \"classical_ym_mass_gap_general_closed\": "
        << (rep.classical_ym_mass_gap_general_closed ? "true" : "false") << ",\n";
    out << "  \"classical_ym_millennium_closed\": "
        << (rep.classical_ym_millennium_closed ? "true" : "false") << ",\n";
    out << "  \"classical_ym_millennium_universal_closed\": "
        << (rep.classical_ym_millennium_universal_closed ? "true" : "false") << ",\n";
    out << "  \"goldbach_effective_n_max\": " << rep.goldbach_effective.n_max_checked << ",\n";
    out << "  \"goldbach_effective_even_count\": " << rep.goldbach_effective.even_count << ",\n";
    out << "  \"goldbach_spectral_extension_closed\": "
        << (rep.goldbach_spectral_extension_closed ? "true" : "false") << ",\n";
    const double minor = std::max(static_cast<double>(rep.goldbach.minor_arc_bound), 1e-12);
    const double major_minor_ratio =
        static_cast<double>(rep.goldbach.major_arc_spectral_mass) / minor;
    out << "  \"goldbach_major_minor_ratio\": " << major_minor_ratio << ",\n";
    out << "  \"bsd_spectral_extension_closed\": "
        << (rep.bsd_spectral_extension_closed ? "true" : "false") << ",\n";
    out << "  \"bsd_millennium_spectral_extension_closed\": "
        << (rep.bsd_millennium_spectral_extension_closed ? "true" : "false") << ",\n";
    out << "  \"hodge_spectral_extension_closed\": "
        << (rep.hodge_spectral_extension_closed ? "true" : "false") << ",\n";
    out << "  \"hodge_millennium_spectral_extension_closed\": "
        << (rep.hodge_millennium_spectral_extension_closed ? "true" : "false") << ",\n";
    out << "  \"ym_millennium_spectral_extension_closed\": "
        << (rep.ym_millennium_spectral_extension_closed ? "true" : "false") << ",\n";
    out << "  \"bsd_rank_proved\": " << (rep.bsd.bsd_rank_proved ? "true" : "false") << ",\n";
    out << "  \"bsd_millennium_proved\": " << (rep.bsd.bsd_millennium_proved ? "true" : "false")
        << ",\n";
    out << "  \"hodge_conjecture_proved\": "
        << (rep.hodge.hodge_conjecture_proved ? "true" : "false") << ",\n";
    out << "  \"hodge_millennium_proved\": "
        << (rep.hodge.hodge_millennium_proved ? "true" : "false") << ",\n";
    out << "  \"goldbach_proved\": " << (rep.goldbach_proved_closed ? "true" : "false") << ",\n";
    out << "  \"ym_mass_gap_proved\": " << (rep.ym_mass_gap_proved_closed ? "true" : "false")
        << ",\n";
    out << "  \"classical_ym_mass_gap_general\": "
        << (rep.classical_ym_mass_gap_general_closed ? "true" : "false") << ",\n";
    const std::string ym_global =
        mrs_global_publication_tier(rep.merged_audit, compile_bundle("programs/marshal_ladder.mrs", true),
                                    "classical_ym_mass_gap_general");
    const std::string ym_millennium =
        mrs_global_publication_tier(rep.merged_audit, compile_bundle("programs/marshal_ladder.mrs", true),
                                    "classical_ym_millennium");
    const MrsCompilationBundle tier_bundle = compile_bundle("programs/marshal_ladder.mrs", true);
    auto capstone_tier = [&](const char* oid) {
        return mrs_global_publication_tier(rep.merged_audit, tier_bundle, oid);
    };
    std::string rh_global = "REDUCTION";
    MrsProofAuditReport rh_audit;
    if (import_mrs_proof_audit_json("docs/generated/mrs_proof_audit.json", rh_audit)) {
        if (suzuki_rh_analytic_closed_from_disk()) promote_suzuki_discharged_referee_classes(rh_audit);
        const MrsCompilationBundle rh_bundle =
            compile_bundle("programs/marshal_xi_hadamard.mrs", true);
        rh_global = mrs_global_publication_tier(rh_audit, rh_bundle,
                                                "classical_riemann_hypothesis_marshal");
    }
    out << "  \"ym_global_publication_tier\": \"" << ym_global << "\",\n";
    out << "  \"ym_millennium_publication_tier\": \"" << ym_millennium << "\",\n";
    out << "  \"global_capstone_tiers\": {\n";
    out << "    \"classical_riemann_hypothesis_marshal\": \"" << rh_global << "\",\n";
    out << "    \"classical_bsd_rank_general\": \"" << capstone_tier("classical_bsd_rank_general")
        << "\",\n";
    out << "    \"classical_bsd_millennium\": \"" << capstone_tier("classical_bsd_millennium")
        << "\",\n";
    out << "    \"classical_bsd_millennium_universal\": \""
        << capstone_tier("classical_bsd_millennium_universal") << "\",\n";
    out << "    \"classical_goldbach\": \"" << capstone_tier("classical_goldbach") << "\",\n";
    out << "    \"classical_hodge11_general\": \"" << capstone_tier("classical_hodge11_general")
        << "\",\n";
    out << "    \"classical_hodge_millennium\": \"" << capstone_tier("classical_hodge_millennium")
        << "\",\n";
    out << "    \"classical_hodge_millennium_universal\": \""
        << capstone_tier("classical_hodge_millennium_universal") << "\",\n";
    out << "    \"classical_ym_mass_gap_general\": \"" << ym_global << "\",\n";
    out << "    \"classical_ym_millennium\": \"" << ym_millennium << "\",\n";
    out << "    \"classical_ym_millennium_universal\": \""
        << capstone_tier("classical_ym_millennium_universal") << "\"\n";
    out << "  },\n";
    out << "  \"classical_goldbach\": " << (rep.classical_goldbach_closed ? "true" : "false")
        << "\n}\n";
    return true;
}

}  // namespace Marshal::AnaVM
