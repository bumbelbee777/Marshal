#pragma once
#include <string>
#include <vector>
#include "Config.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"
#include "Numerics/TestFunctions.hxx"
#include "OperatorCandidateEval.hxx"

namespace Marshal::Induction {

void RunOperatorCandidates(const Config& cfg, const TestFunction& tf,
                           const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
                           Heat::PrimeCatalog& cat);

void ExportOperatorCandidatesJson(const std::string& path,
                                  const std::vector<OperatorCandidateResult>& results,
                                  const Config& cfg, size_t n_zeros);

}  // namespace Marshal::Induction
