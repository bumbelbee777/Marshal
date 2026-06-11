#pragma once
#include "MrsTypes.hxx"
#include <string>
#include <vector>

namespace Marshal::AnaVM {

struct ParseResult {
    bool ok = false;
    MrsProgram program;
    std::vector<MrsError> errors;
};

ParseResult parse_mrs_file(const std::string& path);
void print_errors(const std::vector<MrsError>& errors);

}  // namespace Marshal::AnaVM
