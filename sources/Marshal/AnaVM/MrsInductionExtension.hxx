#pragma once

#include "MrsMath.hxx"
#include "MrsProofTypes.hxx"
#include "MrsTypes.hxx"

#include <string>

namespace Marshal::AnaVM {

struct MrsCompilationBundle;

/// Evaluate Inductive obligation via parsed `induction(...)` prove body.
bool evaluate_induction_witness(const MrsProofObligationDecl& ob,
                                const MrsCompilationBundle& bundle,
                                const MrsMathWitnessEnv& env, std::string* detail,
                                std::string* error);

}  // namespace Marshal::AnaVM
