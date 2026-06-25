#pragma once



#include "AnaProofEngine.hxx"

#include "MrsLadderProofGate.hxx"

#include "MrsProofAudit.hxx"

#include "MrsProveSpine.hxx"

#include "Config.hxx"

#include "Heat/GLn/GL2GoldbachEffectiveValidation.hxx"
#include "Heat/GLn/GL4YMEngine.hxx"



#include <string>

#include <vector>



namespace Marshal::AnaVM {



struct MrsLadderProofEngineReport {

    Heat::GLn::GL2BSDProofReport bsd;

    Heat::GLn::GL3HodgeProofReport hodge;

    Heat::GLn::GL2EllipseHeegnerReport goldbach;

    Heat::GLn::GoldbachEffectiveReport goldbach_effective;

    Heat::GLn::GL4YMProofReport ym;

    MrsProofAuditReport merged_audit;

    bool proof_chain_closed = false;

    bool gl2_l_function_identification_closed = false;

    bool hodge_lefschetz_closed = false;

    bool goldbach_circle_method_closed = false;

    bool classical_goldbach_closed = false;

    bool goldbach_proved_closed = false;

    bool goldbach_spectral_extension_closed = false;

    bool bsd_spectral_extension_closed = false;

    bool bsd_millennium_spectral_extension_closed = false;

    bool hodge_spectral_extension_closed = false;

    bool hodge_millennium_spectral_extension_closed = false;

    bool classical_bsd_rank_general_closed = false;

    bool classical_bsd_millennium_closed = false;

    bool classical_bsd_millennium_universal_closed = false;

    bool classical_hodge11_general_closed = false;

    bool classical_hodge_millennium_closed = false;

    bool classical_hodge_millennium_universal_closed = false;

    bool classical_ym_mass_gap_general_closed = false;

    bool classical_ym_millennium_closed = false;

    bool classical_ym_millennium_universal_closed = false;

    bool ym_millennium_spectral_extension_closed = false;

    bool ym_mass_gap_proved_closed = false;

    MrsProveSpineReport prove_spine;

};



MrsLadderProofEngineReport run_mrs_ladder_proof_engine(const Config& cfg,

                                                       const std::vector<int>& primes);



bool export_mrs_ladder_proof_audit_json(const std::string& path,

                                        const MrsLadderProofEngineReport& rep);



bool export_mrs_ladder_closure_json(const std::string& path,

                                    const MrsLadderProofEngineReport& rep);



}  // namespace Marshal::AnaVM

