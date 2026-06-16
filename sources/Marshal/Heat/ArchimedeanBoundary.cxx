#include "ArchimedeanBoundary.hxx"



#include "Heat/Common.hxx"

#include "TraceApi.hxx"



#include <cmath>



namespace Marshal::Heat {



const char* arch_type_name(AnaVM::ArchimedeanType t) {

    switch (t) {

        case AnaVM::ArchimedeanType::Torus: return "torus";

        case AnaVM::ArchimedeanType::HalfLine: return "half_line";

        default: return "real_line";

    }

}



const char* arch_boundary_name(AnaVM::ArchimedeanBoundary b) {

    switch (b) {

        case AnaVM::ArchimedeanBoundary::Dirichlet: return "dirichlet";

        case AnaVM::ArchimedeanBoundary::Neumann: return "neumann";

        case AnaVM::ArchimedeanBoundary::Periodic: return "periodic";

        default: return "berry_keating";

    }

}



const char* arch_cutoff_name(AnaVM::ArchimedeanCutoff c) {

    return c == AnaVM::ArchimedeanCutoff::ParameterLambda ? "parameter_lambda" : "planck_scale";

}



Real boundary_window(Real t, const ArchimedeanBoundarySpec& spec) {

    if (spec.boundary == AnaVM::ArchimedeanBoundary::BerryKeating) return 1.0L;

    const Real Lambda = std::max(spec.cutoff_lambda, Real{1e-6});

    const Real u = t / Lambda;

    switch (spec.boundary) {

        case AnaVM::ArchimedeanBoundary::Dirichlet:

            return std::tanh(kPi * u);

        case AnaVM::ArchimedeanBoundary::Neumann:

            return 1.0L / std::cosh(kPi * u);

        case AnaVM::ArchimedeanBoundary::Periodic:

            return 1.0L + 0.05L * std::cos(2.0L * kPi * u);

        default:

            return 1.0L;

    }

}



Real archimedean_baseline_for_tf(const TestFunction& tf, Real sigma, SimdLevel simd,

                                 bool precision_mode, int arch_pts, Real eps, bool quick_sinc2) {

    return Marshal::ArchimedeanBaselineForTestFunction(tf, sigma, simd, precision_mode, arch_pts,

                                                       eps, quick_sinc2);

}



Real archimedean_for_tf_bounded(const TestFunction& tf, Real sigma,

                                const ArchimedeanBoundarySpec& spec, Marshal::SimdLevel simd,

                                bool precision_mode, int arch_pts, Real eps, bool quick_sinc2) {

    if (spec.is_baseline() && spec.type == AnaVM::ArchimedeanType::RealLine)

        return archimedean_baseline_for_tf(tf, sigma, simd, precision_mode, arch_pts, eps,

                                           quick_sinc2);



    const Real baseline =

        archimedean_baseline_for_tf(tf, sigma, simd, precision_mode, arch_pts, eps, quick_sinc2);

    Real factor = 1.0L;

    AnaVM::ArchimedeanType effective = spec.type;

    if (spec.type == AnaVM::ArchimedeanType::Torus) effective = AnaVM::ArchimedeanType::RealLine;

    if (effective == AnaVM::ArchimedeanType::HalfLine) factor *= 0.5L;

    switch (spec.boundary) {

        case AnaVM::ArchimedeanBoundary::Dirichlet:

            factor *= 1.05L;

            break;

        case AnaVM::ArchimedeanBoundary::Neumann:

            factor *= 1.03L;

            break;

        case AnaVM::ArchimedeanBoundary::Periodic:

            factor *= 1.04L;

            break;

        default:

            break;

    }

    return baseline * factor;

}



}  // namespace Marshal::Heat

