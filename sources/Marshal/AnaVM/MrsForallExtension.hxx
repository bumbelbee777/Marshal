#pragma once

#include "MrsMath.hxx"
#include "MrsProofTypes.hxx"
#include "MrsTypes.hxx"

#include <string>

namespace Marshal::AnaVM {

struct MrsCompilationBundle;

/// Evaluate Universal obligation via parsed `forall_extension(...)` prove body.
bool evaluate_forall_extension_witness(const MrsProofObligationDecl& ob,
                                       const MrsCompilationBundle& bundle,
                                       const MrsMathWitnessEnv& env, std::string* detail,
                                       std::string* error);

}  // namespace Marshal::AnaVM
