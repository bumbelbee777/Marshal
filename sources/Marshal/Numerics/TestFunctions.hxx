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
    explicit Sinc2Test(Real t) : T(t) {}
    static Real sinc(Real x) {
        if (std::fabsl(x) < 1e-12L) return 1.0L;
        return std::sinl(x) / x;
    }
    Real h(Real t) const override {
        const Real x = t / T;
        const Real s = sinc(x);
        return s * s;
    }
    Real h_hat(Real u) const override {
        const Real au = std::fabsl(u);
        if (au >= 2.0L * kPi / T) return 0.0L;
        const Real w = (1.0L - au * T / (2.0L * kPi)) * kPi;
        return w > 0 ? w : 0.0L;
    }
    Real support_radius() const override { return 3.0L * T; }
    Real hhat_support() const override { return 2.0L * kPi / T; }
    bool has_compact_hhat() const override { return true; }
    const char* name() const override { return "sinc2"; }
};

struct BumpTest : TestFunction {
    Real scale;
    explicit BumpTest(Real s = 1.0L) : scale(s) {}
    Real h(Real t) const override {
        const Real x = t / scale;
        if (std::fabsl(x) >= 1.0L) return 0.0L;
        return std::expl(-1.0L / (1.0L - x * x));
    }
    Real h_hat(Real) const override { return 0.0L; }  // numeric arch only
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

enum class TestKind { Gauss, Sinc2, Bump, Rational };

inline TestKind parse_test_kind(const std::string& s) {
    if (s == "sinc2") return TestKind::Sinc2;
    if (s == "bump") return TestKind::Bump;
    if (s == "rational") return TestKind::Rational;
    return TestKind::Gauss;
}

inline const char* test_kind_name(TestKind k) {
    switch (k) {
        case TestKind::Sinc2: return "sinc2";
        case TestKind::Bump: return "bump";
        case TestKind::Rational: return "rational";
        default: return "gauss";
    }
}
