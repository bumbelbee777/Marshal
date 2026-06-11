#pragma once
#include "MrsTypes.hxx"
#include <optional>
#include <string>

namespace Marshal::AnaVM {

struct SymDeriveResult {
    bool ok = false;
    bool placeholder = false;
    std::string rule_id;
    std::string omega_derived;
    std::string lambda_derived;
};

SymDeriveResult derive_spectrum(const MrsProgram& prog);

bool check_weil_coupling(const MrsProgram& prog, std::vector<MrsError>& errors);

}  // namespace Marshal::AnaVM
