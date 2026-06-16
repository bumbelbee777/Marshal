#pragma once

#include "MrsProofTypes.hxx"

namespace Marshal::AnaVM {

struct MrsKernelReport {
    bool ok = false;
    std::vector<MrsError> errors;
    int proved_count = 0;
    int inferred_count = 0;
};

MrsKernelReport check_compilation_bundle(MrsCompilationBundle& bundle);

}  // namespace Marshal::AnaVM
