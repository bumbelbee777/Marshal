#pragma once

#include <string>
#include <vector>
#include "Config.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"
#include "Numerics/TestFunctions.hxx"
#include "TraceApi.hxx"

namespace Marshal::Induction {

struct WeilConvergencePoint {
    int n_zeros = 0;
    int n_primes = 0;
    int p_max = 0;
    Real weil_residual = 0;
    Real zero_sum = 0;
    Real prime = 0;
    Real arch = 0;
    Real poles = 0;
};

struct WeilConvergenceResult {
    Real T = 0;
    Real kappa = 0;
    std::vector<WeilConvergencePoint> zero_ladder;
    std::vector<WeilConvergencePoint> prime_ladder;
};

struct ArchKappaVerifyPoint {
    Real kappa = 0;
    Real adaptive_arch = 0;
};

struct ArchSinc2AuditResult {
    Real T = 0;
    Real kappa = 0;
    std::vector<Marshal::ArchSinc2AuditPoint> points;
    Real adaptive_arch = 0;
    std::vector<ArchKappaVerifyPoint> kappa_verify;
    Marshal::ArchSinc2ConvergeResult converge;
    bool used_converge = false;
};

ArchSinc2AuditResult RunArchSinc2Audit(const Config& cfg);
ArchSinc2AuditResult RunArchSinc2Converge(const Config& cfg);

WeilConvergenceResult RunWeilConvergenceStudy(const Config& cfg,
                                              const std::vector<double>& gammas,
                                              const std::vector<Real>& gammas_ld,
                                              Heat::PrimeCatalog& cat,
                                              const std::vector<int>& primes);

bool ExportArchSinc2AuditJson(const std::string& path, const ArchSinc2AuditResult& r);
bool ExportWeilConvergenceJson(const std::string& path, const WeilConvergenceResult& r);

}  // namespace Marshal::Induction
