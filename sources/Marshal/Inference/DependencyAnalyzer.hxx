#pragma once

#include "ProofGraph.hxx"
#include <set>
#include <vector>

namespace Marshal::Inference {

struct BlockedNode {
    std::string id;
    std::vector<std::string> depends_on;
    bool all_dependencies_satisfied = false;
};

struct FalsifiedNode {
    std::string id;
    std::vector<std::string> blocked_verdicts;
};

struct NumericalGap {
    std::string gate;
    double current = 0;
    double target = 0;
    std::string prediction;
};

struct UnreachableNode {
    std::string id;
    std::string blocked_by;
};

struct AnalysisResult {
    std::vector<BlockedNode> blocked_ready;
    std::vector<BlockedNode> blocked_waiting;
    std::vector<FalsifiedNode> falsified;
    std::vector<NumericalGap> numerical_gaps;
    std::vector<UnreachableNode> unreachable;
};

inline bool has_falsified_dependency(const ProofGraph& g, const std::string& id,
                                     std::string& blocker,
                                     std::set<std::string>& visiting) {
    if (visiting.count(id)) return false;
    visiting.insert(id);
    const auto it = g.nodes.find(id);
    if (it == g.nodes.end()) {
        visiting.erase(id);
        return false;
    }
    if (it->second.status == ProofStatus::Falsified ||
        it->second.status == ProofStatus::Impossible) {
        blocker = id;
        visiting.erase(id);
        return true;
    }
    for (const std::string& dep : it->second.depends_on) {
        if (has_falsified_dependency(g, dep, blocker, visiting)) {
            visiting.erase(id);
            return true;
        }
    }
    visiting.erase(id);
    return false;
}

inline bool all_deps_satisfied(const ProofGraph& g, const ProofNode& n) {
    for (const std::string& dep : n.depends_on) {
        const auto it = g.nodes.find(dep);
        if (it == g.nodes.end()) continue;
        if (!dependency_satisfied(it->second.status)) return false;
    }
    return true;
}

inline AnalysisResult analyze_dependencies(const ProofGraph& g) {
    AnalysisResult r;
    for (const auto& [id, node] : g.nodes) {
        if (node.status == ProofStatus::Falsified) {
            FalsifiedNode fn;
            fn.id = id;
            fn.blocked_verdicts = node.blocks;
            if (fn.blocked_verdicts.empty()) fn.blocked_verdicts.push_back("SPECTRUM_IDENTIFIED");
            r.falsified.push_back(std::move(fn));
        }
        if (node.status == ProofStatus::Open) {
            BlockedNode bn;
            bn.id = id;
            bn.depends_on = node.depends_on;
            bn.all_dependencies_satisfied = all_deps_satisfied(g, node);
            if (bn.all_dependencies_satisfied)
                r.blocked_ready.push_back(std::move(bn));
            else
                r.blocked_waiting.push_back(std::move(bn));
        }
        if (node.status == ProofStatus::Open || node.status == ProofStatus::Numerical) {
            std::string blocker;
            std::set<std::string> visiting;
            if (has_falsified_dependency(g, id, blocker, visiting)) {
                UnreachableNode u;
                u.id = id;
                u.blocked_by = blocker;
                r.unreachable.push_back(std::move(u));
            }
        }
    }

    if (g.cert.quotient_gamma_tuned_gap > 0.1 && g.cert.quotient_gamma_tuned_gap < 1.0) {
        NumericalGap ng;
        ng.gate = "quotient_max_gap";
        ng.current = g.cert.quotient_gamma_tuned_gap;
        ng.target = 0.1;
        ng.prediction = "will_fail_at_tighter_threshold";
        r.numerical_gaps.push_back(std::move(ng));
    }
    if (g.cert.compact_sinc2_mismatch_proved && g.cert.compact_sinc2_residual > 1.0) {
        NumericalGap ng;
        ng.gate = "compact_sinc2";
        ng.current = g.cert.compact_sinc2_residual;
        ng.target = 1e-10;
        ng.prediction = "falsified";
        r.numerical_gaps.push_back(std::move(ng));
    }
    return r;
}

}  // namespace Marshal::Inference
