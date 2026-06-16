#include "GLnArchimedeanOperator.hxx"

#include "BerryKeatingOperator.hxx"
#include "Heat/Common.hxx"

#include <cmath>

namespace Marshal::Heat::GLn {

namespace {

constexpr int kHodgeH11 = 20;

}  // namespace

std::vector<Real> build_gln_archimedean_ladder(const GLnArchimedeanSpec& spec) {
    if (spec.rank <= 1 || spec.preset == GLnArchPreset::BerryKeating) {
        BerryKeatingSpec bspec = spec.bk;
        bspec.theta = spec.theta;
        auto ladder = bk_wkb_ladder(bspec, false);
        if (static_cast<int>(ladder.size()) > spec.arch_cap)
            ladder.resize(static_cast<size_t>(spec.arch_cap));
        return ladder;
    }
    if (spec.rank == 2 && spec.preset == GLnArchPreset::MaassH2) {
        std::vector<Real> ladder;
        const int n = std::max(10, std::min(spec.arch_cap, 120));
        ladder.reserve(static_cast<size_t>(n));
        for (int j = 1; j <= n; ++j) {
            const Real t = static_cast<Real>(j) * 3.14159265358979323846L / spec.theta;
            const Real lambda = 0.25L + t * t;
            ladder.push_back(std::sqrt(std::max(lambda, Real{0})));
        }
        return ladder;
    }
    if (spec.rank == 2 && spec.preset == GLnArchPreset::MaassEllipseHeegner) {
        std::vector<Real> ladder;
        ladder.push_back(1e-8L);
        const int n = std::max(10, std::min(spec.arch_cap, 100));
        for (int j = 1; j <= n; ++j) {
            const Real t = static_cast<Real>(j) * 3.14159265358979323846L / spec.theta;
            const Real lambda = 0.25L + t * t;
            ladder.push_back(std::sqrt(std::max(lambda, Real{0})));
        }
        return ladder;
    }
    if (spec.rank == 3 && spec.preset == GLnArchPreset::HitchinK3Stub) {
        std::vector<Real> ladder;
        ladder.reserve(static_cast<size_t>(kHodgeH11 + 12));
        ladder.push_back(0.01L);
        for (int j = 0; j < kHodgeH11; ++j)
            ladder.push_back(1e-12L * static_cast<Real>(j + 1));
        ladder.push_back(0.01L);
        const int n_excited = std::max(8, std::min(spec.arch_cap - static_cast<int>(ladder.size()), 40));
        for (int j = 1; j <= n_excited; ++j)
            ladder.push_back(static_cast<Real>(j) * 3.0L);
        return ladder;
    }
    if (spec.rank >= 4 && spec.preset == GLnArchPreset::CliffordStub) {
        std::vector<Real> ladder;
        const int block_mults[4] = {1, 3, 3, 1};
        Real base = spec.theta > 0 ? spec.theta : 5.76L;
        for (int b = 0; b < 4; ++b) {
            for (int j = 0; j < block_mults[b]; ++j) {
                const Real t = static_cast<Real>(b + 1) + 0.25L * static_cast<Real>(j + 1);
                ladder.push_back(std::sqrt(base * t));
            }
        }
        const int extra = std::max(0, std::min(spec.arch_cap - static_cast<int>(ladder.size()), 32));
        for (int j = 1; j <= extra; ++j)
            ladder.push_back(static_cast<Real>(j) * static_cast<Real>(spec.rank) * 2.0L);
        return ladder;
    }
    std::vector<Real> ladder;
    const int n = std::max(8, std::min(spec.arch_cap, 80));
    for (int j = 1; j <= n; ++j)
        ladder.push_back(static_cast<Real>(j) * static_cast<Real>(spec.rank));
    return ladder;
}

}  // namespace Marshal::Heat::GLn
