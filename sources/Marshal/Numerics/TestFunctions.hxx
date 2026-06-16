#pragma once
// Pluggable test functions for Weil explicit formula.
#include <cmath>
#include <string>

#include "Real.hxx"
extern const Real kPi;
extern const Real kSqrt2Pi;

struct TestFunction {
    virtual ~TestFunction() = default;
    virtual Real h(Real t) const = 0;
    virtual Real h_hat(Real u) const = 0;
    virtual Real support_radius() const = 0;
    virtual Real hhat_support() const = 0;
    virtual bool has_compact_hhat() const { return false; }
    virtual const char* name() const = 0;
};

struct GaussTest : TestFunction {
    Real sigma;
    explicit GaussTest(Real s) : sigma(s) {}
    Real h(Real t) const override { return std::expl(-t * t / (2.0L * sigma * sigma)); }
    Real h_hat(Real u) const override {
        return sigma * kSqrt2Pi * std::expl(-0.5L * sigma * sigma * u * u);
    }
    Real support_radius() const override { return sigma * std::sqrtl(-2.0L * std::logl(1e-16L)); }
    Real hhat_support() const override { return 1e300L; }
    const char* name() const override { return "gauss"; }
};

struct Sinc2Test : TestFunction {
    Real T;
    Real kappa;
    explicit Sinc2Test(Real t, Real k = 1.0L) : T(t), kappa(k > 0 ? k : 1.0L) {}
    static Real sinc(Real x) {
        if (std::fabsl(x) < 1e-12L) return 1.0L;
        return std::sinl(x) / x;
    }
    static Real sinc_sq_weil(Real x) {
        const Real s = sinc(x);
        return s * s;
    }
    Real h(Real t) const override {
        const Real x = kappa * t / T;
        return sinc_sq_weil(x);
    }
    Real h_hat(Real u) const override {
        const Real au = std::fabsl(u);
        const Real sup = kPi * kappa / T;
        if (au >= sup) return 0.0L;
        const Real w = (T / kappa) * (1.0L - au * T / (kPi * kappa));
        return w > 0 ? w : 0.0L;
    }
    Real support_radius() const override { return 3.0L * T / kappa; }
    Real hhat_support() const override { return kPi * kappa / T; }
    bool has_compact_hhat() const override { return true; }
    const char* name() const override { return "sinc2"; }
};

inline Real suggest_sinc2_kappa(Real T, int p_max) {
    if (T <= 0 || p_max <= 1) return 1.0L;
    const Real lp = std::log(static_cast<Real>(p_max));
    return std::ceill(T * lp / kPi);
}

struct BumpTest : TestFunction {
    Real scale;
    explicit BumpTest(Real s = 1.0L) : scale(s) {}
    Real h(Real t) const override {
        const Real x = t / scale;
        if (std::fabsl(x) >= 1.0L) return 0.0L;
        return std::expl(-1.0L / (1.0L - x * x));
    }
    Real h_hat(Real u) const override {
        if (std::fabsl(u) > 200.0L / scale) return 0.0L;
        const int n_pts = 2001;
        const Real L = scale;
        const Real dx = 2.0L * L / static_cast<Real>(n_pts - 1);
        Real sum = 0.0L;
        for (int i = 0; i < n_pts; ++i) {
            const Real t = -L + static_cast<Real>(i) * dx;
            const Real w = (i == 0 || i == n_pts - 1) ? 1.0L : ((i & 1) ? 4.0L : 2.0L);
            sum += w * h(t) * std::cosl(u * t);
        }
        return (dx / 3.0L) * sum;
    }
    Real support_radius() const override { return scale; }
    Real hhat_support() const override { return 1e300L; }
    const char* name() const override { return "bump"; }
};

struct RationalTest : TestFunction {
    Real a;
    explicit RationalTest(Real a_) : a(a_) {}
    Real h(Real t) const override { return 1.0L / (t * t + a * a); }
    Real h_hat(Real u) const override {
        return (kPi / a) * std::expl(-a * std::fabsl(u));
    }
    Real support_radius() const override { return 20.0L * a; }
    Real hhat_support() const override { return 1e300L; }
    const char* name() const override { return "rational"; }
};

struct LaplaceTest : TestFunction {
    Real a;
    explicit LaplaceTest(Real a_) : a(a_) {}
    Real h(Real t) const override { return std::expl(-a * std::fabsl(t)); }
    Real h_hat(Real u) const override { return 2.0L * a / (a * a + u * u); }
    Real support_radius() const override { return 20.0L / a; }
    Real hhat_support() const override { return 1e300L; }
    const char* name() const override { return "laplace"; }
};

enum class TestKind { Gauss, Sinc2, Bump, Rational, Laplace };

inline TestKind parse_test_kind(const std::string& s) {
    if (s == "sinc2") return TestKind::Sinc2;
    if (s == "bump") return TestKind::Bump;
    if (s == "rational") return TestKind::Rational;
    if (s == "laplace") return TestKind::Laplace;
    return TestKind::Gauss;
}

inline const char* test_kind_name(TestKind k) {
    switch (k) {
        case TestKind::Sinc2: return "sinc2";
        case TestKind::Bump: return "bump";
        case TestKind::Rational: return "rational";
        case TestKind::Laplace: return "laplace";
        default: return "gauss";
    }
}
