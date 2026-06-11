#pragma once
#include <vector>
#include "Common.hxx"
#include "PrimeCatalog.hxx"
#include "Numerics/TestFunctions.hxx"

namespace Marshal::Heat {

struct HeatCylinderOp {
    int prime = 0;
    Real logp = 0;
    Real inv_logp = 0;
    Real sqrtp = 0;

    HeatCylinderOp() = default;
    HeatCylinderOp(int p, Real lp, Real ilp, Real sp);
    explicit HeatCylinderOp(const PrimeCatalog& cat, size_t idx);

    Real heat_trace_modes(Real t, int nmax) const;
    Real heat_trace_theta(Real t, int kmax) const;
    Real ab_heat_block(Real t, int kmax, Real eps) const;
    Real prime_block_raw(const TestFunction& tf, int kmax, Real eps) const;
    Real log_euler_spectral_adaptive(Real s, Real eps) const;
    static Real log_euler_analytic(int p, Real s);

    struct TracePacket {
        Real prime_norm = 0;
        Real heat_norm = 0;
        Real poisson_err = 0;
        Real euler_err = 0;
    };

    TracePacket trace_packet(const TestFunction& tf, Real tau, int kmax, int nmax,
                             int ktheta, Real s_euler, Real eps, Real link_n) const;
    std::vector<Real> eigenvalues(int count) const;
    Real heat_kernel_trace(Real t, int nmax, int ktheta) const;
};

struct LocalAssembly {
    static size_t clamp_count(const PrimeCatalog& cat, size_t count);
    static void partial_sums(int nmax, int ktheta, Real s_euler, Real eps,
                             const TestFunction& tf, const PrimeCatalog& cat, size_t count,
                             Real tau, Real link_n, Real& prime_out, Real& heat_out,
                             Real& max_poisson, Real& max_prime_heat, Real& max_euler);
};

}  // namespace Marshal::Heat
