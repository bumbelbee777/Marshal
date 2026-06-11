#pragma once
#include <string>
#include <vector>

namespace Marshal::AnaVM {

struct SourceSpan {
    std::string file;
    int line = 0;
    int col = 0;
};

enum class SymTier { Production, Scaffold };

enum class HeatCoupling { None, Poisson, BerryKeating, ConnesHeat };

enum class SymRuleStatus { Implemented, Partial, Placeholder };

struct MrsError {
    std::string code;
    SourceSpan span;
    std::string message;
    std::string hint;
};

struct RescaleSpec {
    bool has_derivative_scale = false;
    std::string derivative_factor;  // e.g. "log(p)"
};

struct OnBlock {
    std::string space;
    std::string omega_expr;
    std::string lambda_expr;
    SourceSpan span;
};

struct FormalRef {
    std::string lemma_id;
    std::string status;  // open | proved | falsified
};

struct MrsProgram {
    std::string id;
    std::string source_path;
    bool uses_gamma = false;
    SymTier sym_tier = SymTier::Production;
    HeatCoupling heat_coupling = HeatCoupling::Poisson;
    std::string kind = "operator";
    RescaleSpec rescale;
    std::vector<OnBlock> on_blocks;
    std::vector<std::string> expects;
    std::vector<FormalRef> lemma_refs;
    bool falsify_sinc2 = false;
    bool trace_lhs_quotient = false;
    bool pair_correlation_gue = false;
    bool formal_analytics = false;
    bool placeholder = false;
    std::string rule_id;
    std::string derived_omega;
    std::string derived_lambda;
    std::string required_omega;
    bool weil_ok = true;
};

}  // namespace Marshal::AnaVM
