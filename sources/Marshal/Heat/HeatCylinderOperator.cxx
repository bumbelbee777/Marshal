#include "HeatCylinderOperator.hxx"

#include "Kernel/FusedHotPaths.hxx"
#include "Kernel/ScaleHotPaths.hxx"
#include "Numerics/Support.hxx"

namespace Marshal::Heat {

HeatCylinderOp::HeatCylinderOp(int p, Real lp, Real ilp, Real sp)
    : prime(p), logp(lp), inv_logp(ilp), sqrtp(sp) {}

HeatCylinderOp::HeatCylinderOp(const PrimeCatalog& cat, size_t idx)
    : prime(cat.p[idx]), logp(cat.logp[idx]),
      inv_logp(cat.inv_logp[idx]), sqrtp(cat.sqrtp[idx]) {}

Real HeatCylinderOp::heat_trace_modes(Real t, int nmax) const {
    Kahan acc;
    acc.add(1.0L);
    acc.add(2.0L * Kernel::FusedHeatTracePoisson(t, inv_logp, nmax));
    return acc.total() * inv_logp;
}

Real HeatCylinderOp::heat_trace_theta(Real t, int kmax) const {
    Kahan acc;
    const Real inv4t = 1.0L / (4.0L * t);
    const Real lp2 = logp * logp;
    for (int k = 1; k <= kmax; ++k) {
        acc.add(2.0L * std::exp(-static_cast<Real>(k * k) * lp2 * inv4t));
    }
    acc.add(1.0L);
    return acc.total() / std::sqrt(4.0L * kPi * t);
}

Real HeatCylinderOp::ab_heat_block(Real t, int kmax, Real eps) const {
    return Kernel::FusedAbHeatBlock(t, logp, kmax, eps);
}

Real HeatCylinderOp::prime_block_raw(const TestFunction& tf, int kmax, Real eps) const {
    Kahan acc;
    Real ppow = sqrtp;
    for (int k = 1; k <= kmax; ++k) {
        const Real u = static_cast<Real>(k) * logp;
        const Real term = (logp / ppow) * 2.0L * tf.h_hat(u);
        acc.add(term);
        if (term < eps) {
            break;
        }
        ppow *= sqrtp;
    }
    return acc.total();
}

Real HeatCylinderOp::log_euler_spectral_adaptive(Real s, Real eps) const {
    Kahan acc;
    for (int k = 1; k < 100000; ++k) {
        const Real term = std::exp(-static_cast<Real>(k) * s * logp) / static_cast<Real>(k);
        acc.add(term);
        if (term < eps) {
            break;
        }
    }
    return acc.total();
}

Real HeatCylinderOp::log_euler_analytic(int p, Real s) {
    const Real ps = std::exp(-s * std::log(static_cast<Real>(p)));
    return -std::log(1.0L - ps);
}

HeatCylinderOp::TracePacket HeatCylinderOp::trace_packet(
    const TestFunction& tf, Real tau, int kmax, int nmax, int ktheta, Real s_euler, Real eps,
    Real link_n) const {
    TracePacket pk;
    const int nm = std::min(nmax, Marshal::Numerics::AdaptiveNmax(inv_logp, tau, eps));
    const int kt = std::min(ktheta, Marshal::Numerics::AdaptiveKtheta(logp, tau, eps));
    const Real modes = heat_trace_modes(tau, nm);
    const Real theta = heat_trace_theta(tau, kt);
    pk.poisson_err = std::fabs(modes - theta);
    pk.prime_norm = prime_block_raw(tf, kmax, eps) / (2.0L * kPi);
    pk.heat_norm = ab_heat_block(tau, kmax, eps) * link_n;
    const Real es = log_euler_spectral_adaptive(s_euler, eps);
    pk.euler_err = std::fabs(es - log_euler_analytic(prime, s_euler));
    return pk;
}

std::vector<Real> HeatCylinderOp::eigenvalues(int count) const {
    std::vector<Real> ev;
    ev.reserve(static_cast<size_t>(count));
    for (int n = 1; n <= count; ++n) {
        ev.push_back(2.0L * kPi * static_cast<Real>(n) * inv_logp);
    }
    return ev;
}

Real HeatCylinderOp::heat_kernel_trace(Real t, int nmax, int ktheta) const {
    const Real modes = heat_trace_modes(t, nmax);
    const Real theta = heat_trace_theta(t, ktheta);
    return 0.5L * (modes + theta);
}

size_t LocalAssembly::clamp_count(const PrimeCatalog& cat, size_t count) {
    if (count == 0 || count > cat.p.size()) {
        return cat.p.size();
    }
    return count;
}

void LocalAssembly::partial_sums(int nmax, int ktheta, Real s_euler, Real eps,
                                 const TestFunction& tf, const PrimeCatalog& cat, size_t count,
                                 Real tau, Real link_n, Real& prime_out, Real& heat_out,
                                 Real& max_poisson, Real& max_prime_heat, Real& max_euler) {
    count = clamp_count(cat, count);
    prime_out = heat_out = 0;
    max_poisson = max_prime_heat = max_euler = 0;
    for (size_t i = 0; i < count; ++i) {
        HeatCylinderOp op(cat, i);
        const int km = cat.kmax_adaptive[i];
        const auto pk = op.trace_packet(tf, tau, km, nmax, ktheta, s_euler, eps, link_n);
        prime_out += pk.prime_norm;
        heat_out += pk.heat_norm;
        max_poisson = std::max(max_poisson, pk.poisson_err);
        max_prime_heat = std::max(max_prime_heat, std::fabs(pk.prime_norm - pk.heat_norm));
        max_euler = std::max(max_euler, pk.euler_err);
    }
}

}  // namespace Marshal::Heat
