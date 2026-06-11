#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include "../Numerics/Real.hxx"

namespace Marshal::Kernel {

constexpr int kZeroChunk = 1 << 16;
constexpr int kPrimeBatch = 512;

struct alignas(64) PrimeBlockSoA {
    std::vector<int> primes;
    std::vector<double> log_p;
    std::vector<double> inv_sqrt_p;
};

struct ZeroOracleSoA {
    std::vector<double> gamma;
    std::vector<Real> gamma_ld;

    size_t ChunkCount() const {
        return (gamma.size() + static_cast<size_t>(kZeroChunk) - 1) /
               static_cast<size_t>(kZeroChunk);
    }
};

}  // namespace Marshal::Kernel
