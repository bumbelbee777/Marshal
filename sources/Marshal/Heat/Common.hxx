#pragma once
#include "Numerics/Real.hxx"

namespace Marshal::Heat {

inline constexpr Real kPi = MARSHAL_LIT(3.141592653589793238462643383279502884);

struct Kahan {
    Real sum = 0.0L;
    Real c = 0.0L;
    void add(Real x) noexcept {
        const Real y = x - c;
        const Real t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    Real total() const noexcept { return sum; }
};

}  // namespace Marshal::Heat
