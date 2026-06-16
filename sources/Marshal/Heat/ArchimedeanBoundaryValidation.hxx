#pragma once



#include <string>

#include <vector>



#include "ArchimedeanBoundary.hxx"

#include "Config.hxx"

#include "Numerics/Real.hxx"



namespace Marshal::Heat {



struct ArchimedeanTestRow {

    std::string name;

    Real weil_residual = 0;

    Real t1_gap = 0;

    bool weil_pass = false;

    bool t1_pass = false;

};



struct ArchimedeanBoundaryRow {

    AnaVM::ArchimedeanType type = AnaVM::ArchimedeanType::RealLine;

    AnaVM::ArchimedeanBoundary boundary = AnaVM::ArchimedeanBoundary::BerryKeating;

    AnaVM::ArchimedeanCutoff cutoff = AnaVM::ArchimedeanCutoff::PlanckScale;

    Real lambda = kArchPlanckScale;

    ArchImplementation implementation = ArchImplementation::Preliminary;

    std::string fallback;

    std::vector<ArchimedeanTestRow> tests;

    bool all_tests_pass = false;

    Real max_weil_residual = 0;

};



struct ArchimedeanBoundaryReport {

    std::string program_id;

    bool sweep_mode = false;

    std::vector<ArchimedeanBoundaryRow> rows;

    int best_row_index = -1;

    std::string verdict;

};



ArchimedeanBoundaryReport run_archimedean_boundary_validation(

    const Config& cfg, const std::vector<double>& gammas, const std::vector<Real>& gammas_ld,

    const std::vector<int>& primes);



bool export_archimedean_boundary_json(const std::string& path,

                                      const ArchimedeanBoundaryReport& r);



}  // namespace Marshal::Heat

