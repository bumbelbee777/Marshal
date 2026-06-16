#pragma once

#include "XiHadamardEngine.hxx"

#include <complex>
#include <vector>

namespace Marshal::Heat {

/// C++ audit spine for unconditional classical RH (MarshalWedgeClosure route).
struct RhUnconditionalAudit {
    bool off_height_log_summability_ok = false;
    bool infinite_det_eq_riemann_xi_off_forced_ok = false;
    bool off_forced_xi_nonzero_ok = false;
    bool critical_line_zero_structure_ok = false;
    bool xi_zero_classification_ok = false;
    bool classical_rh_ok = false;
    double max_off_forced_rel_gap = 0;
    int rh_zero_audit_count = 0;
};

RhUnconditionalAudit audit_marshal_rh_unconditional(const XiHadamardReport& rep,
                                                    const std::vector<double>& ident_gammas,
                                                    const std::vector<int>& primes,
                                                    std::complex<double> mult, bool wedge_ident_ok);

}  // namespace Marshal::Heat
