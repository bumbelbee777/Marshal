#include "Heat/ArchimedeanBoundary.hxx"
#include "Heat/LogPrimeOperator.hxx"
#include "Numerics/TestFunctions.hxx"
#include "TraceApi.hxx"

#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    using namespace Marshal;
    using namespace Marshal::Heat;

    const GaussTest gauss(5.0L);
    const ArchimedeanBoundarySpec baseline{
        AnaVM::ArchimedeanType::RealLine, AnaVM::ArchimedeanBoundary::BerryKeating,
        AnaVM::ArchimedeanCutoff::PlanckScale, kArchPlanckScale};
    const Real arch_base =
        archimedean_baseline_for_tf(gauss, 5.0L, SimdLevel::Scalar, true, 400000, 1e-30L);
    const Real arch_bounded =
        archimedean_for_tf_bounded(gauss, 5.0L, baseline, SimdLevel::Scalar, true, 400000, 1e-30L);
    assert(std::fabs(arch_base - arch_bounded) < 1e-10L * std::max(Real{1}, std::fabs(arch_base)));

    ArchimedeanBoundarySpec dirichlet = baseline;
    dirichlet.boundary = AnaVM::ArchimedeanBoundary::Dirichlet;
    const Real arch_dir =
        archimedean_for_tf_bounded(gauss, 5.0L, dirichlet, SimdLevel::Scalar, true, 400000, 1e-30L);
    assert(std::fabs(arch_dir - arch_base) > 1e-12L * std::max(Real{1}, std::fabs(arch_base)));

    ArchimedeanBoundarySpec torus = baseline;
    torus.type = AnaVM::ArchimedeanType::Torus;
    assert(torus.implementation() == ArchImplementation::Scaffold);
    const Real arch_torus =
        archimedean_for_tf_bounded(gauss, 5.0L, torus, SimdLevel::Scalar, true, 400000, 1e-30L);
    assert(std::isfinite(static_cast<double>(arch_torus)));

    const LogPrimeOperator op2 = LogPrimeOperator::from_prime(2);
    const LogPrimeOperator op3 = LogPrimeOperator::from_prime(3);
    assert(std::fabs(op2.eigenvalue(1) - op3.eigenvalue(1)) > 0.01L);

    std::cout << "ArchimedeanBoundaryTest OK\n";
    return 0;
}
