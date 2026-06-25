#pragma once

#include "MrsProofTypes.hxx"

#include <string>
#include <vector>

namespace Marshal::AnaVM {

struct ProofObligation {
    std::string id;
    std::string statement;
    std::vector<std::string> dependencies;
    ProofClass proof_class = ProofClass::Numeric;
    ProofStatus status = ProofStatus::Pending;
    std::string evidence;
    std::string failure_reason;
};

struct ProofGraphReport {
    std::string target_theorem = "classical_riemann_hypothesis_marshal_proved";
    std::string architecture = "acyclic_marshal_hadamard";
    bool acyclic = true;
    bool all_proved = false;
    bool circular_logic_detected = false;
    std::vector<std::string> cycle_path;
    std::vector<ProofObligation> obligations;
    std::vector<std::string> proved_ids;
    std::vector<std::string> failed_ids;
    std::vector<std::string> topological_order;
};

ProofGraphReport build_marshal_hadamard_proof_graph();

ProofGraphReport proof_graph_from_mrs_bundle(const MrsCompilationBundle& bundle);

ProofGraphReport proof_graph_from_mrs_bundle_named(const MrsCompilationBundle& bundle,
                                                   const std::string& graph_name);

ProofGraphReport build_marshal_hadamard_proof_graph_from_mrs(const std::string& entry_path);

bool proof_graph_has_cycle(const ProofGraphReport& g, std::vector<std::string>* cycle_out);

std::vector<std::string> proof_graph_topological_order(const ProofGraphReport& g);

void apply_numeric_evidence(ProofGraphReport& g, const std::string& id, bool ok,
                            const std::string& evidence, const std::string& fail = {});

void apply_structural_evidence(ProofGraphReport& g, const std::string& id,
                               const std::string& evidence);

ProofGraphReport finalize_proof_graph(ProofGraphReport g);

bool export_proof_graph_json(const std::string& path, const ProofGraphReport& g);

}  // namespace Marshal::AnaVM
