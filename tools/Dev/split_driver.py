#!/usr/bin/env python3
from __future__ import annotations
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
lines = (ROOT / "sources/Marshal/MarshalDriver.cxx").read_text(encoding="utf-8").splitlines()

def sub(s):
    reps = [
        ("WeilResult", "TraceResult"), ("EvaluateWeil", "EvaluateTrace"),
        ("WeilApi.hxx", "TraceApi.hxx"), ("run_evaluate", "RunEvaluate"),
        ("sigma_weil", "sigma_trace"), ("weil_block_raw", "prime_block_raw"),
        ("pk.weil_norm", "pk.prime_norm"), ("v.local_weil", "v.local_prime"),
        ("local_weil_heat", "local_prime_heat"), ("CONTROLLED_WEIL", "CONTROLLED_TRACE"),
        ("Weil / heat", "Marshal trace"), ("Trace-Weil", "Trace-formula"),
        ("Weil(H_", "Trace(H_"), ("Weil=AB", "Prime=AB"), ("Weil identity", "trace identity"),
        ("Weil(P)", "Prime(P)"), ("weil_ok", "trace_ok"), ("tf_weil", "tf_trace"),
        ("cfg_weil", "cfg_trace"), ("global_weil", "global_trace"),
        ("cum_weil_prefix", "cum_prime_prefix"), ("max_weil_heat", "max_prime_heat"),
        (".weilcache", ".zerocache"), ("kCacheMagic = 0x5A4C494557ULL", "kCacheMagic = 0x5A4C5A45434ULL"),
        ("Marshal::Heat::SievePrimes", "Marshal::Heat::LoadOrSievePrimes"),
        ("static void run_", "void Run"), ("static bool parse_", "bool Parse"),
        ("static void print_", "void Print"), ("static std::string hp_", "std::string Hp"),
        ("static void export_", "void Export"), ("static HpProofVerdict run_hp", "HpProofVerdict RunHp"),
        ("static SpectralHpReport compute_", "SpectralHpReport Compute"),
        ("static ResidualBudget compute_", "ResidualBudget Compute"),
        ("static void fill_", "void Fill"), ("static Real mean_", "Real Mean"),
        ("static std::vector<Real> collect_", "std::vector<Real> Collect"),
        ("static weil_toy::", "weil_toy::"), ("static constexpr", "constexpr"),
        ("static const char*", "const char*"), ("static Real gamma_", "Real Gamma_"),
        ("static size_t count_", "size_t Count_"), ("static std::unique_ptr", "std::unique_ptr"),
        ("bool load_zeros_fast", "bool LoadZerosFast"), ("static void promote_zeros_ld", "void PromoteZerosLd"),
        ("tau_from_sigma", "TauFromSigma"),
    ]
    for a,b in reps: s = s.replace(a,b)
    return s

def slice(a, b):
    i = next(x for x,l in enumerate(lines) if re.search(a, l))
    j = next(x for x,l in enumerate(lines[i+1:], i+1) if re.search(b, l))
    return "\n".join(lines[i:j])

IND = ROOT / "sources/Induction"
IND.mkdir(parents=True, exist_ok=True)

hdr = '''#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Analysis/ConvergenceStudy.hxx"
#include "Config.hxx"
#include "Heat/HeatTraceSweep.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Numerics/Real.hxx"
#include "Numerics/TestFunctions.hxx"
#include "TraceApi.hxx"

namespace Marshal::Induction {

inline Real TauFromSigma(Real sigma) noexcept { return 1.0L / (2.0L * sigma * sigma); }

std::unique_ptr<TestFunction> MakeTestFunction(const Config& cfg);
TraceResult RunEvaluate(const Config& cfg, const TestFunction& tf,
                        const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
                        const Heat::PrimeCatalog& cat);
ResidualBudget ComputeResidualBudget(const TestFunction& tf, Real sigma,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    const Heat::PrimeCatalog& cat, ZeroKernel zk, SimdLevel simd,
    bool precision_mode, int arch_pts);

void RunHeatInduction(const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    const Heat::PrimeCatalog& cat);
void RunResidualScaling(const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    const Heat::PrimeCatalog& cat, const std::vector<int>& primes);
void RunHpAnsatz(const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    const Heat::PrimeCatalog& cat, const std::vector<int>& primes);

struct SpectralHpReport {
    Real trace_oracle_lhs = 0;
    Real trace_formula_residual = 0;
    Real max_eigenvalue_gap = 0;
    Real mean_eigenvalue_gap = 0;
    Real direct_sum_max_gap = 0;
    Real direct_sum_mean_gap = 0;
    Real quotient_max_gap = 0;
    Real quotient_mean_gap = 0;
    Real max_gap_in_spacings = 0;
    Real mean_gap_in_spacings = 0;
    int n_pairs = 0;
    int quotient_mesh = 0;
    int quotient_k_primes = 0;
    int quotient_ncells = 0;
    size_t n_effective_zeros = 0;
    bool quotient_skipped = false;
    bool quotient_converged = false;
    int quotient_k_prev = 0;
    Real quotient_gap_prev_k = 0;
    std::string quotient_method = "continuum_haar_rayleigh";
    std::string quotient_k_selection = "fixed";
    bool lhs_underflow = false;
    bool trace_proved = false;
    bool spectrum_approximated = false;
    bool spectrum_identified = false;
    bool locked_spectrum_pass = false;
    bool prony_spectrum_pass = false;
    Real locked_spectrum_max_gap = 0;
    Real locked_spectrum_mean_gap = 0;
    Real prony_spectrum_max_gap = 0;
    Real prony_spectrum_mean_gap = 0;
    bool spec_trace_pass = false;
    bool spec_h_equals_gamma_n = false;
    bool spectral_mismatch = false;
    Heat::HeatTraceSweepResult heat_sweep;
};

struct HpProofVerdict {
    bool local_self_adjoint = true;
    bool base_case = true;
    bool tier1_all = true;
    bool inductive_step = true;
    bool local_prime_heat = true;
    bool global_balance = true;
    bool spectrum_symmetry = true;
    bool local_hp_proved = false;
    bool spec_trace_pass = false;
    bool lhs_underflow = false;
    bool spectral_mismatch = false;
    Real max_eigenvalue_gap = 0;
    Real max_local_err = 0;
    Real global_residual = 0;
    Real ladder_max_err = 0;
    Real local_prime = 0;
    Real local_heat = 0;
    Real local_prime_heat_err = 0;
    Real local_geometric_residual = 0;
    size_t local_prime_count = 0;
    int p_max_local = 0;
    int tier1_failures = 0;
    int failures = 0;
    Real trace_formula_residual = 0;
    Real proof_eps_used = 0;
    bool machine_zero_pass = false;
    Real residual_fp_delta = 0;
};

HpProofVerdict RunHpProofInduction(const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    Heat::PrimeCatalog& cat);

void ExportTraceJson(const std::string& path, const Config& cfg, const TestFunction& tf,
    const TraceResult& r, const Heat::PrimeCatalog& cat, const ResidualBudget& b, size_t n_zeros);
void ExportInductionJson(const std::string& path, const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    const Heat::PrimeCatalog& cat);
void ExportSpectralJson(const std::string& path, int n, const Heat::PrimeCatalog& cat,
    const std::vector<double>& gammas);

bool ParseConfig(int argc, char** argv, Config& cfg, std::string& err);
void PrintUsage(const char* prog);
void PrintResult(Real sigma, const TraceResult& r);
void RunSweep(const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    Heat::PrimeCatalog& cat, const std::vector<int>& primes);
void RunMachineTest(const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    const Heat::PrimeCatalog& cat);
void RunCompactTest(const Config& cfg, const TestFunction& tf,
    const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,
    const Heat::PrimeCatalog& cat);

}  // namespace Marshal::Induction
'''
(IND / "Induction.hxx").write_text(hdr, encoding="utf-8")

# body: arch/zero helpers + induction pipeline through run_compact_test
start = next(i for i,l in enumerate(lines) if l.startswith("inline Real h_gauss"))
end = next(i for i,l in enumerate(lines) if l.startswith("int main("))
body = "\n".join(lines[start:end])
body = sub(body)
body = body.replace("static ", "")

lut = '''
const Real kPi          = 3.141592653589793238462643383279502884L;
const Real kSqrt2Pi     = 2.506628274631000502415165145310982062L;
constexpr Real kEulerGamma  = 0.577215664901532860606512090082402431L;
#include "psi_lut.inc"
#include "gh64.inc"
#include "gh128.inc"
#include "gh256.inc"
#include "gh512.inc"
#include "gh1024.inc"
'''

cpp = '''#include "Induction.hxx"
#include "Compat.hxx"
#include "Cert/Schema.hxx"
#include "Cert/Verdict.hxx"
#include "Diagnostics/TraceModeDiagnostic.hxx"
#include "Heat/HeatCylinderOperator.hxx"
#include "Heat/HeatTraceSweep.hxx"
#include "Quotient/QuotientToy.hxx"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace Marshal::Induction {
namespace {
''' + lut + '''
}

''' + body + "\n}  // namespace Marshal::Induction\n"

(IND / "Induction.cxx").write_text(cpp, encoding="utf-8")

# zero loader
zstart = next(i for i,l in enumerate(lines) if "cache_path_for" in l)
zend = next(i for i,l in enumerate(lines) if "make_test_function" in l)
zl = sub("\n".join(lines[zstart:zend]))
IO = ROOT / "sources/Marshal/IO"
IO.mkdir(exist_ok=True)
(IO / "ZeroLoader.hxx").write_text(
    '#pragma once\n#include <string>\n#include <vector>\n#include "Numerics/Real.hxx"\n'
    'bool LoadZerosFast(const std::string& path, std::vector<double>& out, size_t max_count, bool use_cache, std::vector<Real>* out_ld = nullptr);\n'
    'void PromoteZerosLd(const std::vector<double>& in, std::vector<Real>& out);\n',
    encoding="utf-8")
(IO / "ZeroLoader.cxx").write_text(
    '#include "IO/ZeroLoader.hxx"\n#include "Compat.hxx"\n#include <fstream>\n\n' +
    zl.replace("static ", "").replace("load_zeros_fast", "LoadZerosFast").replace("promote_zeros_ld", "PromoteZerosLd"),
    encoding="utf-8")

# slim main
main = "\n".join(lines[end:])
main = sub(main)
main = '''// MarshalDriver.cxx
#include <atomic>
#include <iostream>
#include <memory>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif
#include "Compat.hxx"
#include "Config.hxx"
#include "TraceApi.hxx"
#include "Heat/PrimeCatalog.hxx"
#include "Heat/PrimeCache.hxx"
#include "IO/ZeroLoader.hxx"
#include "Induction/Induction.hxx"

using Marshal::Config;
using Marshal::Heat::PrimeCatalog;

''' + main
main = main.replace("load_zeros_fast", "LoadZerosFast")
main = main.replace("promote_zeros_ld", "PromoteZerosLd")
main = main.replace("parse_config", "Marshal::Induction::ParseConfig")
main = main.replace("print_usage", "Marshal::Induction::PrintUsage")
main = main.replace("make_test_function", "Marshal::Induction::MakeTestFunction")
main = main.replace("run_evaluate", "Marshal::Induction::RunEvaluate")
main = main.replace("compute_residual_budget", "Marshal::Induction::ComputeResidualBudget")
def pascal(fn):
    parts = fn.split('_')
    return parts[0].capitalize() + ''.join(p.capitalize() for p in parts[1:])
for fn in ["export_trace_json","export_induction_json","export_spectral_json","run_compact_test",
           "run_hp_proof_induction","run_hp_ansatz","run_residual_scaling","run_heat_induction",
           "run_sweep","run_machine_test","print_result"]:
    main = main.replace(fn, f"Marshal::Induction::{pascal(fn)}")
main = main.replace("tau_from_sigma", "Marshal::Induction::TauFromSigma")

(ROOT / "sources/Marshal/MarshalDriver.cxx").write_text(main, encoding="utf-8")
print("split ok", len(cpp), "induction bytes")
