#include "DiagnosticExport.hxx"

#include <fstream>
#include <iomanip>

namespace Marshal::Investigation {

bool export_diagnostic_json(const std::string& path, const DiagnosticReport& r) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(17);
    out << "{\n  \"version\": " << r.version << ",\n";
    out << "  \"investigation_id\": \"" << r.investigation_id << "\",\n";
    out << "  \"diagnostic_id\": \"" << r.diagnostic_id << "\",\n";
    out << "  \"proof_status\": \"" << r.proof_status << "\",\n";
    out << "  \"analysis_status\": \"" << r.analysis_status << "\",\n";
    out << "  \"epistemic_note\": \"" << r.epistemic_note << "\",\n";
    out << "  \"analytic_obligation\": \"" << r.analytic_obligation << "\",\n";
    out << "  \"fixed_theta\": " << static_cast<double>(r.fixed_theta) << ",\n";
    out << "  \"prime_limit\": " << r.prime_limit << ",\n";
    if (!r.note.empty()) out << "  \"note\": \"" << r.note << "\",\n";
    out << "  \"gates\": [\n";
    for (size_t i = 0; i < r.gates.size(); ++i) {
        const auto& g = r.gates[i];
        out << "    {\"id\": \"" << g.id << "\", \"gate\": \"" << g.gate_class
            << "\", \"pass\": " << (g.pass ? "true" : "false") << ", \"note\": \"" << g.note
            << "\"}";
        if (i + 1 < r.gates.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n  \"series\": [\n";
    for (size_t i = 0; i < r.series.size(); ++i) {
        const auto& s = r.series[i];
        out << "    {\"x\": " << static_cast<double>(s.x) << ", \"y\": "
            << static_cast<double>(s.y) << ", \"label\": \"" << s.label
            << "\", \"flag\": " << (s.flag ? "true" : "false") << ", \"aux\": "
            << static_cast<double>(s.aux) << "}";
        if (i + 1 < r.series.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

bool export_investigation_manifest(const std::string& path,
                                   const std::string& investigation_id,
                                   const std::vector<std::pair<std::string, bool>>& entries) {
    std::ofstream out(path);
    if (!out) return false;
    out << "{\n  \"version\": 1,\n";
    out << "  \"investigation_id\": \"" << investigation_id << "\",\n";
    out << "  \"diagnostics\": [\n";
    for (size_t i = 0; i < entries.size(); ++i) {
        out << "    {\"id\": \"" << entries[i].first << "\", \"pass\": "
            << (entries[i].second ? "true" : "false") << "}";
        if (i + 1 < entries.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

}  // namespace Marshal::Investigation
