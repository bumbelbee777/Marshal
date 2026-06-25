#include "FormalCalibration.hxx"

#include <fstream>
#include <iomanip>

namespace Marshal::AnaVM {

FormalCalibration build_formal_calibration(const MrsProgram& prog) {
    FormalCalibration c;
    c.ansatz_id = prog.id;
    c.rule_id = prog.rule_id;
    c.placeholder = prog.placeholder;
    c.scaffold = prog.sym_tier == SymTier::Scaffold;
    c.derived_omega = prog.derived_omega;
    c.derived_lambda = prog.derived_lambda;
    c.lemma_refs = prog.lemma_refs;
    c.falsify_sinc2 = prog.falsify_sinc2;
    c.trace_lhs_quotient = prog.trace_lhs_quotient;
    c.pair_correlation_gue = prog.pair_correlation_gue;
    c.formal_analytics = prog.formal_analytics;
    if (prog.rule_id == "connes_analytic_construction") {
        c.mrs_module = "programs/connes_analytic_construction.mrs";
        c.mrs_cert = "connes_analytic_construction";
    } else if (prog.rule_id == "connes_analytic_lemmas") {
        c.mrs_module = "programs/connes_analytic_lemmas.mrs";
        c.mrs_cert = "connes_analytic_lemmas";
    } else if (prog.rule_id == "connes_global_dirac_limit") {
        c.mrs_module = "programs/connes_global_dirac_limit.mrs";
        c.mrs_cert = "global_connes_limit";
    } else if (prog.rule_id == "spectral_action_selection") {
        c.mrs_module = "programs/spectral_action_selection.mrs";
        c.mrs_cert = "spectral_action_selection";
    } else if (prog.rule_id == "theorem_a_analytic") {
        c.mrs_module = "programs/lib/marshal_theorem_a_proof.mrs";
        c.mrs_cert = "theorem_a";
    } else if (prog.rule_id == "theorem_b") {
        c.mrs_module = "programs/investigations/theorem_b.mrs";
        c.mrs_cert = "theorem_b_scaffold";
    } else if (prog.rule_id == "berry_keating_xp" && !prog.placeholder) {
        c.mrs_module = "programs/berry_keating.mrs";
        c.mrs_cert = "berry_keating";
    } else if (prog.placeholder) {
        if (prog.rule_id == "connes_dirac" || prog.id.find("connes") != std::string::npos)
            c.mrs_module = "programs/connes_crossed_product.mrs";
        else if (prog.rule_id == "berry_keating_xp" || prog.id.find("berry") != std::string::npos)
            c.mrs_module = "programs/berry_keating.mrs";
        else
            c.mrs_module = "programs/cylinder_direct_sum.mrs";
    } else if (prog.pair_correlation_gue || prog.formal_analytics) {
        c.mrs_module = "programs/cylinder_direct_sum.mrs";
    } else {
        c.mrs_module = "programs/cylinder_direct_sum.mrs";
    }
    return c;
}

void export_formal_calibration_json(const std::string& path, const FormalCalibration& cal,
                                    const std::string& cert_path) {
    std::ofstream out(path);
    out << std::setprecision(17);
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"ansatz_id\": \"" << cal.ansatz_id << "\",\n";
    out << "  \"rule_id\": \"" << cal.rule_id << "\",\n";
    out << "  \"placeholder\": " << (cal.placeholder ? "true" : "false") << ",\n";
    out << "  \"scaffold\": " << (cal.scaffold ? "true" : "false") << ",\n";
    out << "  \"derived_omega\": \"" << cal.derived_omega << "\",\n";
    out << "  \"derived_lambda\": \"" << cal.derived_lambda << "\",\n";
    out << "  \"mrs_module\": \"" << cal.mrs_module << "\",\n";
    if (!cal.mrs_cert.empty())
        out << "  \"mrs_cert\": \"" << cal.mrs_cert << "\",\n";
    out << "  \"falsify_sinc2\": " << (cal.falsify_sinc2 ? "true" : "false") << ",\n";
    out << "  \"trace_lhs_quotient\": " << (cal.trace_lhs_quotient ? "true" : "false") << ",\n";
    out << "  \"pair_correlation_gue\": " << (cal.pair_correlation_gue ? "true" : "false") << ",\n";
    out << "  \"formal_analytics\": " << (cal.formal_analytics ? "true" : "false") << ",\n";
    out << "  \"mrs_emit_ready\": " << (cal.mrs_emit_ready ? "true" : "false") << ",\n";
    if (!cert_path.empty()) out << "  \"hp_cert\": \"" << cert_path << "\",\n";
    out << "  \"lemma_refs\": [";
    for (size_t i = 0; i < cal.lemma_refs.size(); ++i) {
        if (i) out << ", ";
        out << "{\"id\": \"" << cal.lemma_refs[i].lemma_id << "\", \"status\": \""
            << cal.lemma_refs[i].status << "\"}";
    }
    out << "]\n";
    out << "}\n";
}

}  // namespace Marshal::AnaVM
