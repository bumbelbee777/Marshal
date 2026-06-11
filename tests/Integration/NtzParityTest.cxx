#include <cstdio>
#include <cmath>
#include "Marshal/Ntz/ZeroOracle.hxx"

int main() {
    Marshal::Ntz::ZeroOracle oracle;
    const char* path = "tests/Fixtures/Zeros/odlyzko_zeros100k.txt";
    if (!oracle.Load(path, 100)) {
        std::fprintf(stderr, "SKIP NtzParityTest: cannot load %s\n", path);
        return 0;
    }
    if (std::fabs(oracle.gamma[0] - 14.134725) > 1e-5) {
        std::fprintf(stderr, "FAIL first zero got %.12f\n", oracle.gamma[0]);
        return 1;
    }
    std::printf("NtzParityTest OK: loaded %zu zeros, gamma_1=%.12f\n",
                oracle.Count(), oracle.gamma[0]);
    return 0;
}
