#pragma once



#include "AnaVM/MrsTypes.hxx"

#include "Numerics/Real.hxx"

#include "Numerics/TestFunctions.hxx"

#include "Config.hxx"



namespace Marshal::Heat {



constexpr Real kArchPlanckScale = 6.283185307179586476925286766559005768394338798750L;



enum class ArchImplementation { Preliminary, Scaffold };



struct ArchimedeanBoundarySpec {

    AnaVM::ArchimedeanType type = AnaVM::ArchimedeanType::RealLine;

    AnaVM::ArchimedeanBoundary boundary = AnaVM::ArchimedeanBoundary::BerryKeating;

    AnaVM::ArchimedeanCutoff cutoff = AnaVM::ArchimedeanCutoff::PlanckScale;

    Real cutoff_lambda = kArchPlanckScale;



    static ArchimedeanBoundarySpec from_mrs(const AnaVM::MrsArchimedean& m) {

        ArchimedeanBoundarySpec s;

        s.type = m.type;

        s.boundary = m.boundary;

        s.cutoff = m.cutoff;

        s.cutoff_lambda = m.cutoff == AnaVM::ArchimedeanCutoff::PlanckScale

                              ? kArchPlanckScale

                              : static_cast<Real>(m.lambda_cutoff);

        return s;

    }



    bool is_baseline() const {

        return type == AnaVM::ArchimedeanType::RealLine &&

               boundary == AnaVM::ArchimedeanBoundary::BerryKeating &&

               cutoff == AnaVM::ArchimedeanCutoff::PlanckScale;

    }



    ArchImplementation implementation() const {

        if (type == AnaVM::ArchimedeanType::Torus) return ArchImplementation::Scaffold;

        return ArchImplementation::Preliminary;

    }

};



Real archimedean_baseline_for_tf(const TestFunction& tf, Real sigma, Marshal::SimdLevel simd,

                                 bool precision_mode, int arch_pts, Real eps,

                                 bool quick_sinc2 = false);



Real archimedean_for_tf_bounded(const TestFunction& tf, Real sigma,

                                const ArchimedeanBoundarySpec& spec, Marshal::SimdLevel simd,

                                bool precision_mode, int arch_pts, Real eps,

                                bool quick_sinc2 = false);



Real boundary_window(Real t, const ArchimedeanBoundarySpec& spec);



const char* arch_type_name(AnaVM::ArchimedeanType t);

const char* arch_boundary_name(AnaVM::ArchimedeanBoundary b);

const char* arch_cutoff_name(AnaVM::ArchimedeanCutoff c);



}  // namespace Marshal::Heat

