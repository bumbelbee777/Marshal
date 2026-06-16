#include "CombinedConnesDirac.hxx"

#include "MarshalGLnDirac.hxx"

namespace Marshal::Heat {

Real combined_spectral_action_heat(const std::vector<Real>& eigenvalues, Real scale_base,
                                   int n_scales) {
    return GLn::gln_spectral_action_heat(eigenvalues, scale_base, n_scales);
}

CombinedConnesDiracResult build_combined_dirac_spectrum(const CombinedConnesDiracSpec& spec,
                                                        const std::vector<int>& primes) {
    CombinedConnesDiracResult out;
    const GLn::MarshalGLnDiracSpec gln = GLn::marshal_gln_spec_from_combined(spec).with_rank(1);
    const GLn::MarshalGLnDiracResult r = GLn::build_gln_dirac_spectrum(gln, primes);
    out.eigenvalues = r.eigenvalues;
    out.n_arch = r.n_arch;
    out.n_prime = r.n_prime;
    out.spectral_action_heat = r.spectral_action_heat;
    return out;
}

}  // namespace Marshal::Heat
