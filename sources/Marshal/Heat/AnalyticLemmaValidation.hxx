#pragma once



#include "AnalyticConstructionValidation.hxx"

#include "Config.hxx"

#include "SpectralActionValidation.hxx"



#include <string>

#include <vector>



namespace Marshal::Heat {



struct LemmaDemonstration {

    std::string lemma_id;

    std::string proof_status;

    std::string gate_id;

    std::string verdict;

    std::string demonstration_class = "numeric";

    bool lean_ready = true;

};



struct AnalyticLemmaReport {

    std::string program_id;

    std::string rule_id;

    std::string proof_status = "ANALYTIC_DEMONSTRATION_OPEN";

    std::string v1_chain_status = "OPEN";

    bool lean_emit_ready = false;

    std::string formal_lemma;

    std::string formal_approach;

    std::vector<LemmaDemonstration> lemmas;

    std::vector<std::string> proved_lemmas;

    std::vector<std::string> open_obligations;

    AnalyticConstructionReport construction;

    SpectralActionReport spectral_action;

    bool extension_selection_proved = false;

    bool spectral_discreteness_proved = false;

};



AnalyticLemmaReport run_analytic_lemma_demo(const Config& cfg,

                                            const std::vector<double>& gammas,

                                            const std::vector<Real>& gammas_ld,

                                            PrimeCatalog& cat,

                                            const std::vector<int>& primes);



bool export_analytic_lemma_demo_json(const std::string& path, const AnalyticLemmaReport& r);



}  // namespace Marshal::Heat

