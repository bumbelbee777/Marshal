#pragma once

#include <string>

namespace Marshal::Heat {

enum class ConnesCouplingMode { PrimePower, LogLadder };

inline const char* connes_coupling_mode_name(ConnesCouplingMode m) {
    switch (m) {
        case ConnesCouplingMode::LogLadder:
            return "log_ladder";
        default:
            return "prime_power";
    }
}

inline ConnesCouplingMode parse_connes_coupling_mode(const std::string& s) {
    if (s == "log_ladder" || s == "log-ladder" || s == "ladder") return ConnesCouplingMode::LogLadder;
    return ConnesCouplingMode::PrimePower;
}

}  // namespace Marshal::Heat
