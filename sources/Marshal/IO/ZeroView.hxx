#pragma once
#include <cstddef>
#include <vector>
#include "Numerics/Real.hxx"

namespace Marshal::IO {

// Zero ordinates for trace evaluation — vector or mmap-backed.
struct ZeroView {
    const double* data = nullptr;
    size_t count = 0;
    std::vector<double> owned;
    std::vector<Real> owned_ld;
    bool mmap_backed = false;

    size_t size() const { return count; }
    bool empty() const { return count == 0; }
    const double* ptr() const { return data ? data : owned.data(); }
    const std::vector<double>& vec() const { return owned; }
    const std::vector<Real>& vec_ld() const { return owned_ld; }

    void Bind(const double* p, size_t n) {
        owned.clear();
        owned_ld.clear();
        data = p;
        count = n;
        mmap_backed = true;
    }

    void LoadVector(std::vector<double>&& g, std::vector<Real>&& ld = {}) {
        owned = std::move(g);
        owned_ld = std::move(ld);
        data = owned.data();
        count = owned.size();
        mmap_backed = false;
    }

    const Real* ld_ptr() const { return owned_ld.empty() ? nullptr : owned_ld.data(); }
    size_t ld_count() const { return owned_ld.size(); }
};

}  // namespace Marshal::IO
