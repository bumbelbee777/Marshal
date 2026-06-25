#pragma once

#include "MrsMath.hxx"
#include "MrsProofTypes.hxx"

#include <string>

namespace Marshal::AnaVM {

struct MrsCompilationBundle;

/// Evaluate Convergent obligation via parsed `convergence(...)` prove body.
bool evaluate_convergence_witness(const MrsProofObligationDecl& ob,
                                  const MrsCompilationBundle& bundle,
                                  const MrsMathWitnessEnv& env, std::string* detail,
                                  std::string* error);

}  // namespace Marshal::AnaVM
