#include "QuotientToy.hxx"
#include "Heat/PrimeSieve.hxx"

#include <cstring>

namespace weil_toy {

std::vector<int> sieve_primes(int max_n) {
    return Marshal::Heat::SievePrimes(max_n);
}

static bool parse_zero_line(const char* b, const char* e, double& val) {
    return parse_zero_line_fc(b, e, val);
}

bool load_zeros(const std::string& path, std::vector<double>& out, size_t max_count) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) {
        std::cerr << "Cannot open zero file: " << path << "\n";
        return false;
    }
    const std::streamsize sz = in.tellg();
    if (sz <= 0) return false;
    in.seekg(0);
    std::vector<char> blob(static_cast<size_t>(sz) + 1);
    in.read(blob.data(), sz);
    blob[static_cast<size_t>(sz)] = '\0';

    out.clear();
    if (max_count > 0) out.reserve(max_count);

    const char* p = blob.data();
    const char* end = blob.data() + sz;
    while (p < end) {
        const char* line = p;
        while (p < end && *p != '\n' && *p != '\r') ++p;
        while (p < end && (*p == '\n' || *p == '\r')) ++p;
        const char* b = line;
        while (b < p && (*b == ' ' || *b == '\t')) ++b;
        double v = 0;
        if (parse_zero_line(b, p, v)) {
            out.push_back(v);
            if (max_count > 0 && out.size() >= max_count) break;
        }
    }
    if (out.empty()) {
        std::cerr << "No zeros parsed from: " << path << "\n";
        return false;
    }
    return true;
}

}  // namespace weil_toy
