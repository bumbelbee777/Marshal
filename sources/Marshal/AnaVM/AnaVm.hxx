#pragma once
#include "MrsParser.hxx"
#include <string>

namespace Marshal::AnaVM {

struct CompileResult {
    bool ok = false;
    MrsProgram program;
    std::vector<MrsError> errors;
};

CompileResult compile_program(const std::string& path, bool check_only = false);

}  // namespace Marshal::AnaVM
