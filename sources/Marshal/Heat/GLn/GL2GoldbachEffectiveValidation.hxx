#pragma once

namespace Marshal::Heat::GLn {

struct GoldbachEffectiveReport {
    int n0 = 4;
    int n_max_checked = 10000;
    int even_count = 0;
    int failure_n = 0;
    bool ok = false;
};

/// Direct prime-pair check for even n in [n0, n_max] (Goldbach effective range).
GoldbachEffectiveReport run_goldbach_effective_validation(int n0, int n_max);

}  // namespace Marshal::Heat::GLn
