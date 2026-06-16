#pragma once

#include <string>
#include <vector>

#include "AnaVM/MrsTypes.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::Heat {

struct HeightMapSpec {
    AnaVM::HeightMapType type = AnaVM::HeightMapType::Log;
    Real a = 1.0L;
    Real b = 0.0L;
    Real alpha = 1.0L;
    std::string formula;

    static HeightMapSpec from_mrs(const AnaVM::MrsHeightMap& m);
};

Real apply_height_map(Real omega, const HeightMapSpec& spec);

void apply_height_map_inplace(std::vector<Real>& freqs, const HeightMapSpec& spec);

std::vector<Real> height_lut(const std::vector<int>& primes, int kmax, const HeightMapSpec& spec);

}  // namespace Marshal::Heat
