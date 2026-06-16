#pragma once

#include "MrsProofTypes.hxx"

#include <string>

namespace Marshal::AnaVM {

MrsCompilationUnit parse_mrs_unit(const std::string& path);

}  // namespace Marshal::AnaVM
