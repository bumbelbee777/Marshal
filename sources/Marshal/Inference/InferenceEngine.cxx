#include "InferenceEngine.hxx"

#include "ActionSuggester.hxx"
#include "DependencyAnalyzer.hxx"
#include "ProofGraph.hxx"

#include <algorithm>
#include <iostream>

namespace Marshal::Inference {

bool run_inference(const InferenceConfig& cfg, std::string& err) {
    const std::string manifest_text = read_text_file(cfg.manifest_path);
    if (manifest_text.empty()) {
        err = "Cannot read manifest: " + cfg.manifest_path;
        return false;
    }
    const std::string ansatz_text = read_text_file(cfg.ansatz_registry_path);
    if (ansatz_text.empty()) {
        err = "Cannot read ansatz registry: " + cfg.ansatz_registry_path;
        return false;
    }
    const std::string cert_text = cfg.cert_path.empty() ? std::string{} : read_text_file(cfg.cert_path);

    const ProofGraph graph =
        build_proof_graph(manifest_text, ansatz_text, cert_text, cfg.manifest_path);
    const AnalysisResult analysis = analyze_dependencies(graph);
    const std::vector<NextAction> actions = suggest_actions(graph, analysis);

    const std::string verdict = inference_verdict(graph);
    export_next_actions_json(cfg.export_next_actions_path, verdict, actions);

    std::cout << "Inference: " << actions.size() << " next action(s) -> "
              << cfg.export_next_actions_path << "\n";
    const int show = static_cast<int>(std::min(actions.size(), size_t{5}));
    for (int i = 0; i < show; ++i) {
        const auto& a = actions[static_cast<size_t>(i)];
        std::cout << "  [" << a.priority << "] " << a.action;
        if (!a.lemma.empty()) std::cout << " " << a.lemma;
        if (!a.blocked_by.empty()) std::cout << " (blocked_by=" << a.blocked_by << ")";
        std::cout << "\n";
    }
    return true;
}

}  // namespace Marshal::Inference
