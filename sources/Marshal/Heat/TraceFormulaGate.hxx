#pragma once

#include "Config.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"

#include <string>
#include <vector>

namespace Marshal::Heat {

struct TraceFormulaTestRow {
    std::string name;
    Real lhs = 0;
    Real rhs = 0;
    Real residual = 0;
    bool pass = false;
};

struct TraceFormulaGateReport {
    std::string program_id;
    std::string rule_id;
    Real max_residual = 0;
    Real residual_threshold = 1.0L;
    Real log_prime_t1_gap = 0;
    Real arch_weil_residual = 0;
    bool combined_bk_logprime = false;
    std::vector<TraceFormulaTestRow> tests;
    std::string verdict;
    std::string t1_verdict;
    std::string full_weil_verdict;
};

TraceFormulaGateReport run_trace_formula_gate(const Config& cfg,
                                              const std::vector<double>& gammas,
                                              const std::vector<Real>& gammas_ld,
                                              PrimeCatalog& cat, const std::vector<int>& primes);

bool export_trace_formula_gate_json(const std::string& path, const TraceFormulaGateReport& r);

}  // namespace Marshal::Heat
