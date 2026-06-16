#pragma once
#include "MrsInfer.hxx"
#include "MrsKernel.hxx"
#include "MrsModuleParser.hxx"
#include "MrsParser.hxx"
#include <string>

namespace Marshal::AnaVM {

struct CompileResult {
    bool ok = false;
    MrsProgram program;
    MrsCompilationBundle bundle;
    MrsInferReport infer_report;
    std::vector<MrsError> errors;
};

MrsCompilationBundle compile_bundle(const std::string& entry_path, bool check_only = false,
                                    MrsInferReport* infer_out = nullptr);

CompileResult compile_program(const std::string& path, bool check_only = false);

}  // namespace Marshal::AnaVM
