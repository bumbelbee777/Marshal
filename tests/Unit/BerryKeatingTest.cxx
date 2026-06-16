#include "Heat/BerryKeatingOperator.hxx"

#include <cmath>
#include <iostream>
#include <vector>

using Marshal::Heat::BerryKeatingSpec;
using Marshal::Heat::bk_wkb_eigenvalue;
using Marshal::Heat::bk_wkb_ladder;

int main() {
    int fails = 0;
    auto check = [&](bool ok, const char* msg) {
        if (!ok) {
            std::cerr << "FAIL: " << msg << "\n";
            ++fails;
        }
    };

    const Real g1 = bk_wkb_eigenvalue(1, 0, 6.0L);
    const Real g2 = bk_wkb_eigenvalue(2, 0, 6.0L);
    check(g2 > g1, "WKB ladder monotonic");
    check(g1 > 0, "first WKB mode positive");

    BerryKeatingSpec spec;
    spec.log_span = 6.0L;
    spec.max_modes = 100;
    const auto ladder = bk_wkb_ladder(spec);
    check(ladder.size() == 100, "ladder length");
    for (size_t i = 1; i < ladder.size(); ++i)
        check(ladder[i] > ladder[i - 1], "ladder sorted");

    BerryKeatingSpec spec2 = spec;
    spec2.theta = 1.0L;
    const auto ladder2 = bk_wkb_ladder(spec2);
    check(std::fabs(ladder2[0] - ladder[0]) > 1e-6, "theta shifts spectrum");

    BerryKeatingSpec spec3 = spec;
    spec3.apply_log_height = true;
    const auto mapped = bk_wkb_ladder(spec3);
    const auto raw = bk_wkb_ladder(spec3, false);
    check(mapped[9] < raw[9], "height map reduces high modes");
    check(mapped[0] > 0, "height map n=1 positive");

    if (fails) {
        std::cerr << fails << " test(s) failed\n";
        return 1;
    }
    std::cout << "BerryKeatingTest: ok\n";
    return 0;
}
