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
    if (prog.placeholder) {
        if (prog.rule_id == "connes_dirac" || prog.id.find("connes") != std::string::npos)
            c.lean_module = "AdeleQuotient";
        else if (prog.rule_id == "berry_keating_xp" || prog.id.find("berry") != std::string::npos)
            c.lean_module = "AdeleQuotient";
        else
            c.lean_module = "HPWeil";
    } else if (prog.pair_correlation_gue || prog.formal_analytics) {
        c.lean_module = "CylinderNoGo";
    } else {
        c.lean_module = "HPWeil";
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
    out << "  \"lean_module\": \"" << cal.lean_module << "\",\n";
    out << "  \"falsify_sinc2\": " << (cal.falsify_sinc2 ? "true" : "false") << ",\n";
    out << "  \"trace_lhs_quotient\": " << (cal.trace_lhs_quotient ? "true" : "false") << ",\n";
    out << "  \"pair_correlation_gue\": " << (cal.pair_correlation_gue ? "true" : "false") << ",\n";
    out << "  \"formal_analytics\": " << (cal.formal_analytics ? "true" : "false") << ",\n";
    out << "  \"lean_emit_ready\": " << (cal.lean_emit_ready ? "true" : "false") << ",\n";
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
