#include "IO/ZeroLoader.hxx"
#include "Compat.hxx"
#include <fstream>
#include <iostream>
#ifdef _OPENMP
#include <omp.h>
#endif

void PromoteZerosLd(const std::vector<double>& in, std::vector<Real>& out) {
    out.resize(in.size());
    #ifdef _OPENMP
    #pragma omp parallel for schedule(static)
    #endif
    for (int i = 0; i < static_cast<int>(in.size()); ++i)
        out[static_cast<size_t>(i)] = static_cast<Real>(in[static_cast<size_t>(i)]);
}

std::string CachePathFor(const std::string& src) { return src + ".zerocache"; }

constexpr uint32_t kCacheVersion = 1;
constexpr uint64_t kCacheMagic = 0x5A4C5A45434ULL;

bool LoadZerosBinary(const std::string& path, std::vector<double>& out, size_t max_count) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    uint64_t magic = 0;
    in.read(reinterpret_cast<char*>(&magic), 8);
    if (magic != kCacheMagic) return false;
    uint32_t version = 0, sz = 0;
    uint64_t count = 0;
    uint32_t crc = 0;
    in.read(reinterpret_cast<char*>(&version), 4);
    if (version != kCacheVersion) return false;
    in.read(reinterpret_cast<char*>(&sz), 4);
    if (sz != sizeof(double)) return false;
    in.read(reinterpret_cast<char*>(&count), 8);
    if (count == 0) return false;
    const size_t want = (max_count > 0) ? std::min(max_count, count) : count;
    out.resize(want);
    in.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(want * 8));
    if (static_cast<size_t>(in.gcount()) != want * 8) return false;
    in.read(reinterpret_cast<char*>(&crc), 4);
    return crc == crc32_bytes(out.data(), want * sizeof(double));
}

bool SaveZerosBinary(const std::string& path, const std::vector<double>& data) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    const uint64_t magic = kCacheMagic;
    const uint32_t version = kCacheVersion;
    const uint32_t sz = static_cast<uint32_t>(sizeof(double));
    const uint64_t count = data.size();
    const uint32_t crc = crc32_bytes(data.data(), data.size() * sizeof(double));
    out.write(reinterpret_cast<const char*>(&magic), 8);
    out.write(reinterpret_cast<const char*>(&version), 4);
    out.write(reinterpret_cast<const char*>(&sz), 4);
    out.write(reinterpret_cast<const char*>(&count), 8);
    out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(count * 8));
    out.write(reinterpret_cast<const char*>(&crc), 4);
    return true;
}

bool ParseZeroLine(const char* b, const char* e, double& val) {
    return ParseZeroLineFc(b, e, val);
}

bool LoadZerosTextParallelLd(const std::string& path, std::vector<double>& out,
                                        std::vector<Real>& out_ld, size_t max_count) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) return false;
    const std::streamsize sz = in.tellg();
    if (sz <= 0) return false;
    in.seekg(0);
    std::vector<char> blob(static_cast<size_t>(sz) + 1);
    in.read(blob.data(), sz);
    blob[static_cast<size_t>(sz)] = '\0';

    std::vector<size_t> starts;
    starts.reserve(static_cast<size_t>(sz) / 20 + 16);
    starts.push_back(0);
    for (size_t i = 0; i < static_cast<size_t>(sz); ++i)
        if (blob[i] == '\n') starts.push_back(i + 1);

    const size_t nlines = starts.size();
    std::vector<Real> parsed_ld(nlines);
    std::vector<uint8_t> ok(nlines);

    #ifdef _OPENMP
    #pragma omp parallel for schedule(static)
    #endif
    for (int li = 0; li < static_cast<int>(nlines); ++li) {
        const char* b = blob.data() + starts[static_cast<size_t>(li)];
        const char* e = (li + 1 < static_cast<int>(nlines))
            ? blob.data() + starts[static_cast<size_t>(li + 1)] - 1
            : blob.data() + sz;
        while (b < e && (*b == '\r' || *b == '\n')) ++b;
        Real v = 0;
        if (ParseZeroLineLd(b, e, v)) {
            parsed_ld[static_cast<size_t>(li)] = v;
            ok[static_cast<size_t>(li)] = 1;
        }
    }

    out.clear();
    out_ld.clear();
    if (max_count > 0) {
        out.reserve(max_count);
        out_ld.reserve(max_count);
    } else {
        out.reserve(nlines);
        out_ld.reserve(nlines);
    }
    for (size_t i = 0; i < nlines; ++i) {
        if (!ok[i]) continue;
        out_ld.push_back(parsed_ld[i]);
        out.push_back(static_cast<double>(parsed_ld[i]));
        if (max_count > 0 && out.size() >= max_count) break;
    }
    return !out.empty();
}

bool LoadZerosTextParallel(const std::string& path, std::vector<double>& out, size_t max_count) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) return false;
    const std::streamsize sz = in.tellg();
    if (sz <= 0) return false;
    in.seekg(0);
    std::vector<char> blob(static_cast<size_t>(sz) + 1);
    in.read(blob.data(), sz);
    blob[static_cast<size_t>(sz)] = '\0';

    std::vector<size_t> starts;
    starts.reserve(static_cast<size_t>(sz) / 20 + 16);
    starts.push_back(0);
    for (size_t i = 0; i < static_cast<size_t>(sz); ++i)
        if (blob[i] == '\n') starts.push_back(i + 1);

    const size_t nlines = starts.size();
    std::vector<double> parsed(nlines);
    std::vector<uint8_t> ok(nlines);

    #ifdef _OPENMP
    #pragma omp parallel for schedule(static)
    #endif
    for (int li = 0; li < static_cast<int>(nlines); ++li) {
        const char* b = blob.data() + starts[static_cast<size_t>(li)];
        const char* e = (li + 1 < static_cast<int>(nlines))
            ? blob.data() + starts[static_cast<size_t>(li + 1)] - 1
            : blob.data() + sz;
        while (b < e && (*b == '\r' || *b == '\n')) ++b;
        double v = 0;
        if (ParseZeroLine(b, e, v)) {
            parsed[static_cast<size_t>(li)] = v;
            ok[static_cast<size_t>(li)] = 1;
        }
    }

    out.clear();
    if (max_count > 0) out.reserve(max_count);
    else out.reserve(nlines);
    for (size_t i = 0; i < nlines; ++i) {
        if (!ok[i]) continue;
        out.push_back(parsed[i]);
        if (max_count > 0 && out.size() >= max_count) break;  // checked before push below
    }
    return !out.empty();
}

bool LoadZerosFast(const std::string& path, std::vector<double>& out,
                     size_t max_count, bool use_cache, std::vector<Real>* out_ld) {
    if (out_ld) use_cache = false;
    if (use_cache) {
        std::vector<double> cached;
        if (LoadZerosBinary(CachePathFor(path), cached, 0)) {
            if (max_count == 0 || cached.size() >= max_count) {
                out = std::move(cached);
                if (max_count > 0 && out.size() > max_count) out.resize(max_count);
                if (out_ld) PromoteZerosLd(out, *out_ld);
                return true;
            }
        }
    }
    if (out_ld && LoadZerosTextParallelLd(path, out, *out_ld, max_count))
        return true;
    if (!LoadZerosTextParallel(path, out, max_count)) {
        std::cerr << "Cannot open or parse zero file: " << path << "\n";
        return false;
    }
    if (use_cache) SaveZerosBinary(CachePathFor(path), out);
    return true;
}
