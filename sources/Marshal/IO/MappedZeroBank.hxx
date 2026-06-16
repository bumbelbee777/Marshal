#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include "MappedView.hxx"
#include "Numerics/Real.hxx"

namespace Marshal::IO {

constexpr uint32_t kZeroCacheVersion = 1;
constexpr uint64_t kZeroCacheMagic = 0x5A4C5A45434ULL;

// mmap-backed zero ordinates — no full-vector copy for prefix reads.
struct MappedZeroBank {
    MappedView mv;
    const double* data = nullptr;
    size_t count = 0;
    bool owns_text_parse = false;
    std::vector<double> text_scratch;

    void Unmap() {
        mv.Unmap();
        data = nullptr;
        count = 0;
        text_scratch.clear();
        owns_text_parse = false;
    }

    bool MapBinary(const std::string& path) {
        Unmap();
        if (!mv.Map(path)) return false;
        if (mv.size < 8 + 4 + 4 + 8 + 4) return false;
        const char* p = mv.data;
        uint64_t magic = 0;
        std::memcpy(&magic, p, 8);
        if (magic != kZeroCacheMagic) return false;
        p += 8;
        uint32_t version = 0, sz = 0;
        std::memcpy(&version, p, 4);
        p += 4;
        std::memcpy(&sz, p, 4);
        p += 4;
        if (version != kZeroCacheVersion || sz != sizeof(double)) return false;
        uint64_t n = 0;
        std::memcpy(&n, p, 8);
        p += 8;
        if (n == 0) return false;
        const size_t payload = static_cast<size_t>(n) * sizeof(double);
        if (mv.size < static_cast<size_t>(p - mv.data) + payload + 4) return false;
        data = reinterpret_cast<const double*>(p);
        count = static_cast<size_t>(n);
        return true;
    }

    bool MapText(const std::string& path) {
        Unmap();
        if (!mv.Map(path)) return false;
        const auto offs = ScanLineOffsets(mv);
        text_scratch.clear();
        text_scratch.reserve(offs.size());
        for (size_t i = 0; i + 1 < offs.size(); ++i) {
            double v = 0;
            if (ParseLineView(mv.data + offs[i], mv.data + offs[i + 1], v))
                text_scratch.push_back(v);
        }
        if (text_scratch.empty()) return false;
        data = text_scratch.data();
        count = text_scratch.size();
        owns_text_parse = true;
        return true;
    }

    bool Open(const std::string& path) {
        if (path.size() > 10 && path.compare(path.size() - 10, 10, ".zerocache") == 0)
            return MapBinary(path);
        const std::string cache = path + ".zerocache";
        if (MapBinary(cache)) return true;
        return MapText(path);
    }

    double at(size_t i) const { return i < count ? data[i] : 0.0; }
    size_t size() const { return count; }
    bool empty() const { return count == 0; }

    void CopyPrefix(size_t n, std::vector<double>& out, std::vector<Real>* out_ld = nullptr) const {
        const size_t want = std::min(n, count);
        out.assign(data, data + want);
        if (out_ld) {
            out_ld->resize(want);
            for (size_t i = 0; i < want; ++i)
                (*out_ld)[i] = static_cast<Real>(data[i]);
        }
    }
};

}  // namespace Marshal::IO
