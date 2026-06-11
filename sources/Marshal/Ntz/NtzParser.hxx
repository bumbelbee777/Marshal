#pragma once
#include <cctype>
#include <string>
#include <vector>
#include "../IO/MappedView.hxx"

namespace Marshal::Ntz {

// Parse Odlyzko one-value-per-line or wrapped multi-line high-precision ordinates.
inline bool ParseWrappedOrdinate(const std::vector<std::string>& lines, long double& out) {
    std::string acc;
    for (const auto& ln : lines) {
        for (char c : ln) {
            if (std::isdigit(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == '+')
                acc += c;
        }
    }
    if (acc.empty()) return false;
    char* end = nullptr;
    out = std::strtold(acc.c_str(), &end);
    return end != acc.c_str();
}

inline std::vector<double> LoadZerosOnePerLine(const std::string& path, size_t max_count = 0) {
    std::vector<double> out;
    Marshal::IO::MappedView mv;
    if (!mv.Map(path)) return out;
    const auto offs = Marshal::IO::ScanLineOffsets(mv);
    for (size_t i = 0; i + 1 < offs.size(); ++i) {
        const char* b = mv.data + offs[i];
        const char* e = mv.data + offs[i + 1];
        double v = 0;
        if (Marshal::IO::ParseLineView(b, e, v)) {
            out.push_back(v);
            if (max_count && out.size() >= max_count) break;
        }
    }
    return out;
}

}  // namespace Marshal::Ntz
