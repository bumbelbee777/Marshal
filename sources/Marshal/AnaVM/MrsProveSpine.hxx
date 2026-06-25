#pragma once



#include "MrsProofTypes.hxx"



#include <string>

#include <vector>



namespace Marshal::AnaVM {



struct MrsProveSpineReport {

    bool ok = true;

    bool acyclic = true;

    bool trivial_alias_detected = false;

    bool infer_on_analytic_detected = false;

    std::vector<std::string> trivial_aliases;

    std::vector<std::string> infer_on_analytic;

    std::vector<std::string> prove_cycle_path;

    std::vector<std::string> missing_witness_expr;

    std::vector<std::string> missing_prove_ref;

    std::vector<std::string> undisciplined_prove;

    std::vector<std::string> script_dep_mismatch;

    bool obligation_graph_acyclic = true;

    bool circular_witness_detected = false;

    bool weak_witness_detected = false;

    bool capstone_in_witness_detected = false;

    bool opaque_composition_detected = false;

    bool tautological_prove_detected = false;

    bool circular_identification_detected = false;

    bool weak_analytic_reduction_detected = false;

    bool goal_equality_in_witness_detected = false;

    bool rh_assumption_smuggle_detected = false;

    bool assume_target_leak_detected = false;

    bool missing_explicit_steps_detected = false;

    std::vector<std::string> obligation_cycle_path;

    std::vector<std::string> circular_witness;

    std::vector<std::string> weak_witness;

    std::vector<std::string> capstone_in_witness;

    std::vector<std::string> opaque_composition;

    std::vector<std::string> tautological_prove;

    std::vector<std::string> circular_identification;

    std::vector<std::string> weak_analytic_reduction;

    std::vector<std::string> goal_equality_in_witness;

    std::vector<std::string> rh_assumption_smuggle;

    std::vector<std::string> assume_target_leak;

    std::vector<std::string> missing_explicit_steps;

};



/// Compile-time witness discipline: no self-ref, capstone laundering, or trivial gates.

bool witness_expr_passes_hardening(const std::string& graph_name,
                                   const std::string& obligation_id,
                                   const std::string& witness_expr, std::string* reason);



/// Validate MRS prove spine + obligation dep graph hardening.

std::vector<std::string> prove_body_callees(const std::string& body);



/// Validate prove-body dependency graph and obligation discipline for ladder graphs.

MrsProveSpineReport validate_ladder_prove_spine(const MrsCompilationBundle& bundle);

/// Analytic Hadamard obligations must use assume/conclude prove scripts (not bare lemma aliases).

MrsProveSpineReport validate_hadamard_prove_spine(const MrsCompilationBundle& bundle);

/// Unified discipline gate for all proof graphs (Hadamard + ladder).

MrsProveSpineReport validate_mrs_proof_discipline(const MrsCompilationBundle& bundle);

}  // namespace Marshal::AnaVM

