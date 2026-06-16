#include "HeightMap.hxx"

#include "Heat/Common.hxx"
#include "LogPrimeOperator.hxx"

#include <algorithm>
#include <cmath>

namespace Marshal::Heat {

HeightMapSpec HeightMapSpec::from_mrs(const AnaVM::MrsHeightMap& m) {
    HeightMapSpec s;
    s.type = m.type;
    s.a = static_cast<Real>(m.a);
    s.b = static_cast<Real>(m.b);
    s.alpha = static_cast<Real>(m.alpha);
    s.formula = m.formula;
    return s;
}

Real apply_height_map(Real omega, const HeightMapSpec& spec) {
    const Real w = std::max(omega, Real{1e-30});
    switch (spec.type) {
        case AnaVM::HeightMapType::Power:
            return spec.a * std::pow(w, spec.alpha) + spec.b;
        case AnaVM::HeightMapType::Custom:
            return spec.a * w * std::log(std::max(w, Real{1})) / (2.0L * kPi) + spec.b;
        case AnaVM::HeightMapType::Log:
        default:
            return spec.a * w * std::log(std::max(w, Real{1})) / (2.0L * kPi) + spec.b;
    }
}

void apply_height_map_inplace(std::vector<Real>& freqs, const HeightMapSpec& spec) {
    for (Real& w : freqs) w = apply_height_map(w, spec);
}

std::vector<Real> height_lut(const std::vector<int>& primes, int kmax, const HeightMapSpec& spec) {
    std::vector<Real> lut;
    lut.reserve(primes.size() * static_cast<size_t>(kmax));
    for (int p : primes) {
        const LogPrimeOperator op = LogPrimeOperator::from_prime(p);
        for (int k = 1; k <= kmax; ++k) lut.push_back(apply_height_map(op.eigenvalue(k), spec));
    }
    std::sort(lut.begin(), lut.end());
    return lut;
}

}  // namespace Marshal::Heat
