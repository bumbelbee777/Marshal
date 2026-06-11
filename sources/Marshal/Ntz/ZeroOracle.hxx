#pragma once
#include <string>
#include <vector>
#include "../Numerics/Real.hxx"
#include "NtzParser.hxx"

namespace Marshal::Ntz {

struct ZeroOracle {
    std::vector<double> gamma;
    std::vector<Real> gamma_ld;

    bool Load(const std::string& path, size_t max_count = 0) {
        gamma = LoadZerosOnePerLine(path, max_count);
        gamma_ld.clear();
        gamma_ld.reserve(gamma.size());
        for (double g : gamma) gamma_ld.push_back(static_cast<Real>(g));
        return !gamma.empty();
    }

    size_t Count() const { return gamma.size(); }
    Real MaxGamma() const {
        Real m = 0;
        for (Real g : gamma_ld) if (g > m) m = g;
        return m;
    }
};

}  // namespace Marshal::Ntz
