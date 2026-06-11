#pragma once
// DEPRECATED — frequency-lock is mathematically impossible. See FrequencyLockImpossibility.md.
#include "../Quotient/QuotientToy.hxx"

namespace Marshal::Diagnostics {

inline double LegacyFrequencyLockedSpread(const weil_toy::PrimeList& pl, double gamma) {
    return weil_toy::frequency_locked_spread(pl, gamma);
}

}  // namespace Marshal::Diagnostics
