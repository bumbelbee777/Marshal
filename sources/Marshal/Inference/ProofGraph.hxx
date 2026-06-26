#pragma once

#include "JsonMinimal.hxx"
#include <algorithm>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Marshal::Inference {

enum class ProofStatus {
    Proved,
    Open,
    Falsified,
    Numerical,
    Impossible,
    Unknown
};

inline ProofStatus parse_proof_status(const std::string& s) {
    if (s == "PROVED") return ProofStatus::Proved;
    if (s == "OPEN") return ProofStatus::Open;
    if (s == "FALSIFIED") return ProofStatus::Falsified;
    if (s == "NUMERICAL") return ProofStatus::Numerical;
    if (s == "IMPOSSIBLE" || s == "DISPROVED") return ProofStatus::Impossible;
    return ProofStatus::Unknown;
}

inline const char* proof_status_string(ProofStatus s) {
    switch (s) {
        case ProofStatus::Proved: return "PROVED";
        case ProofStatus::Open: return "OPEN";
        case ProofStatus::Falsified: return "FALSIFIED";
        case ProofStatus::Numerical: return "NUMERICAL";
        case ProofStatus::Impossible: return "IMPOSSIBLE";
        default: return "UNKNOWN";
    }
}

inline bool dependency_satisfied(ProofStatus s) {
    return s == ProofStatus::Proved || s == ProofStatus::Numerical ||
           s == ProofStatus::Falsified;
}

struct ProofNode {
    std::string id;
    ProofStatus status = ProofStatus::Unknown;
    std::vector<std::string> depends_on;
    std::vector<std::string> blocks;
    std::string description;
    std::string doc;
};

struct FalsifiedAnsatz {
    std::string id;
    std::string status;
    std::string program;
    std::string lemma;
};

struct CertSnapshot {
    std::string verdict;
    bool spectrum_identified = false;
    bool compact_sinc2_mismatch_proved = false;
    double compact_sinc2_residual = 0;
    double quotient_gamma_tuned_gap = 0;
    bool local_cylinder_pass = true;
};

struct HuntEvidence {
    bool operator_hunt_pass = false;
    std::string operator_hunt_verdict;
    std::string continuum_verdict;
    std::string analytic_construction_verdict;
    bool scaffold_revised = false;
    bool hunt_closed = false;
};

inline bool hunt_closure_ready(const HuntEvidence& h) {
    return h.operator_hunt_pass && !h.continuum_verdict.empty() &&
           (h.analytic_construction_verdict == "OPEN_SPECTRAL_DISCRETENESS" ||
            h.analytic_construction_verdict == "SPECTRUM_IDENTIFIED_NUMERIC");
}

inline std::string resolve_manifest_evidence(const std::string& manifest_path,
                                             const std::string& rel) {
    if (rel.empty()) return {};
    const auto try_join = [&](const std::string& base) -> std::string {
        if (base.empty()) return rel;
        if (!rel.empty() && (rel[0] == '/' || (rel.size() > 1 && rel[1] == ':'))) return rel;
        char tail = base.back();
        if (tail == '/' || tail == '\\') return base + rel;
        return base + "/" + rel;
    };
    const size_t a = manifest_path.find("/docs/Analysis/");
    if (a != std::string::npos) return try_join(manifest_path.substr(0, a));
    const size_t b = manifest_path.find("\\docs\\Analysis\\");
    if (b != std::string::npos) return try_join(manifest_path.substr(0, b));
    return rel;
}

struct ProofGraph {
    std::map<std::string, ProofNode> nodes;
    std::vector<FalsifiedAnsatz> falsified_ansatze;
    std::vector<std::string> open_ansatze;
    CertSnapshot cert;
    HuntEvidence hunt;
};

inline std::vector<std::string> extract_json_array_objects(const std::string& text,
                                                           const std::string& array_key) {
    std::vector<std::string> blocks;
    const size_t start = text.find(array_key);
    if (start == std::string::npos) return blocks;
    size_t i = text.find('[', start);
    if (i == std::string::npos) return blocks;
    ++i;
    int depth = 0;
    size_t obj_start = std::string::npos;
    for (; i < text.size(); ++i) {
        const char c = text[i];
        if (c == '{') {
            if (depth == 0) obj_start = i;
            ++depth;
        } else if (c == '}') {
            --depth;
            if (depth == 0 && obj_start != std::string::npos) {
                blocks.push_back(text.substr(obj_start, i - obj_start + 1));
                obj_start = std::string::npos;
            }
        } else if (c == ']' && depth == 0) {
            break;
        }
    }
    return blocks;
}

inline ProofGraph build_proof_graph(const std::string& manifest_text,
                                    const std::string& ansatz_text,
                                    const std::string& cert_text,
                                    const std::string& manifest_path = {}) {
    ProofGraph g;
    for (const std::string& block : extract_json_array_objects(manifest_text, "\"lemmas\":")) {
        ProofNode n;
        n.id = json_get_string(block, "id");
        if (n.id.empty()) continue;
        n.status = parse_proof_status(json_get_string(block, "proof_status"));
        n.description = json_get_string(block, "statement");
        n.doc = json_get_string(block, "doc");
        n.depends_on = json_get_string_array(block, "depends_on");
        n.blocks = json_get_string_array(block, "blocks");
        const std::string reduction = json_get_string(block, "proved_reduction");
        if (!reduction.empty()) {
            for (const std::string& tok : split_plus_tokens(reduction)) {
                const std::string norm = normalize_lemma_alias(tok);
                if (std::find(n.depends_on.begin(), n.depends_on.end(), norm) ==
                    n.depends_on.end()) {
                    n.depends_on.push_back(norm);
                }
            }
        }
        g.nodes[n.id] = std::move(n);
    }

    for (const std::string& block : extract_json_array_objects(ansatz_text, "\"ansatze\":")) {
            const std::string id = json_get_string(block, "id");
            const std::string st = json_get_string(block, "status");
            if (id.empty()) continue;
            if (st == "FALSIFIED") {
                FalsifiedAnsatz fa;
                fa.id = id;
                fa.status = st;
                fa.program = json_get_string(block, "program");
                fa.lemma = json_get_string(block, "lemma");
                g.falsified_ansatze.push_back(std::move(fa));
            } else if (st == "OPEN" || st == "CANDIDATE" || st == "DIAGNOSTIC") {
                g.open_ansatze.push_back(id);
            }
    }

    auto hydrate_evidence = [&](const std::string& lemma_id, const std::string& evidence_rel) {
        const std::string path = resolve_manifest_evidence(manifest_path, evidence_rel);
        const std::string text = read_text_file(path);
        if (text.empty()) return;
        if (lemma_id == "operator_hunt_sanity_cert") {
            g.hunt.operator_hunt_verdict = json_get_string(text, "verdict");
            g.hunt.operator_hunt_pass = g.hunt.operator_hunt_verdict == "OPERATOR_HUNT_SANITY_PASS";
        } else if (lemma_id == "continuum_persistence_check") {
            g.hunt.continuum_verdict = json_get_string(text, "verdict");
        } else if (lemma_id == "spectral_discreteness") {
            g.hunt.analytic_construction_verdict = json_get_string(text, "overall_verdict");
            g.hunt.scaffold_revised = !json_get_bool(text, "height_map_applied");
        }
    };

    for (const std::string& block : extract_json_array_objects(manifest_text, "\"lemmas\":")) {
        const std::string id = json_get_string(block, "id");
        const std::string evidence = json_get_string(block, "evidence");
        if (!evidence.empty()) hydrate_evidence(id, evidence);
    }
    if (g.hunt.analytic_construction_verdict.empty()) {
        const std::string path =
            resolve_manifest_evidence(manifest_path, "docs/generated/analytic_construction.json");
        const std::string text = read_text_file(path);
        if (!text.empty()) {
            g.hunt.analytic_construction_verdict = json_get_string(text, "overall_verdict");
            g.hunt.scaffold_revised = !json_get_bool(text, "height_map_applied");
        }
    }
    g.hunt.hunt_closed = hunt_closure_ready(g.hunt);

    if (!cert_text.empty()) {
        g.cert.verdict = json_get_string(cert_text, "verdict");
        g.cert.spectrum_identified = json_get_bool(cert_text, "spectrum_identified");
        g.cert.local_cylinder_pass = json_get_bool(cert_text, "all_pass");
        g.cert.compact_sinc2_residual =
            json_get_double(cert_text, "residual");
        if (g.cert.compact_sinc2_residual == 0)
            g.cert.compact_sinc2_residual =
                json_get_double(cert_text, "compact_sinc2_residual");
        g.cert.compact_sinc2_mismatch_proved =
            json_get_bool(cert_text, "mismatch_proved");
        g.cert.quotient_gamma_tuned_gap =
            json_get_double(cert_text, "quotient_gamma_tuned_sq_gap_max");
        if (g.cert.quotient_gamma_tuned_gap == 0)
            g.cert.quotient_gamma_tuned_gap =
                json_get_double(cert_text, "locked_spectrum_max_gap");
    }
    return g;
}

}  // namespace Marshal::Inference
