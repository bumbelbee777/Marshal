#pragma once
#include "MrsTypes.hxx"
#include <string>
#include <vector>

namespace Marshal::AnaVM {

struct FormalCalibration {
    std::string ansatz_id;
    std::string rule_id;
    bool placeholder = false;
    bool scaffold = false;
    std::string derived_omega;
    std::string derived_lambda;
    std::vector<FormalRef> lemma_refs;
    bool falsify_sinc2 = false;
    bool trace_lhs_quotient = false;
    bool pair_correlation_gue = false;
    bool formal_analytics = false;
    bool lean_emit_ready = false;
    std::string lean_module;  // HPWeil | AdeleQuotient | CylinderNoGo
};

FormalCalibration build_formal_calibration(const MrsProgram& prog);

void export_formal_calibration_json(const std::string& path, const FormalCalibration& cal,
                                    const std::string& cert_path = {});

}  // namespace Marshal::AnaVM
