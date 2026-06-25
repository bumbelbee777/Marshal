// MrsTransform + algebraic normalizer unit tests.

#include "AnaVM/MrsAlgebra.hxx"
#include "AnaVM/MrsTransform.hxx"
#include "AnaVM/MrsProofLogic.hxx"

#include <iostream>

namespace {

int g_fails = 0;

void require(bool ok, const char* msg) {
    if (!ok) {
        std::cerr << "FAIL: " << msg << "\n";
        ++g_fails;
    }
}

}  // namespace

int main() {
    Marshal::AnaVM::MrsMathWitnessEnv env;
    env.nums["a"] = 4;
    env.nums["b"] = 2;

    const std::string n1 = Marshal::AnaVM::algebraic_normalize_expr("1 * a + 0", env);
    require(n1.find("a") != std::string::npos || n1 == "4", "normalize 1*a+0");

    std::string err;
    require(Marshal::AnaVM::algebraic_equiv_expr("ratio(4,2)", "2", env, &err),
            "algebraic_equiv ratio(4,2) == 2");

    const auto db = Marshal::AnaVM::marshal_transform_db();
    require(!db.empty(), "marshal transform db non-empty");

    const Marshal::AnaVM::MrsTransformResult tr = Marshal::AnaVM::apply_mrs_transforms(
        "ratio(a,b)", {"ratio"}, {}, env, {}, &err);
    require(tr.ok && tr.changed, "apply ratio transform");
    require(tr.output.find('/') != std::string::npos, "ratio expands to division");

    const Marshal::AnaVM::MrsProveScript script =
        Marshal::AnaVM::parse_mrs_prove_script(
            "assume:\n  holomorphy_ok\n"
            "transform:\n  transform margin := ratio(a,b) via ratio, normalize\n"
            "conclude:\n  arith_eq(ratio(a,b), margin)");

    require(script.transforms.size() == 1, "parse transform section");
    require(script.transforms[0].hints.size() >= 1, "parse transform hints");

    if (g_fails == 0) {
        std::cout << "MrsTransformTest: all passed\n";
        return 0;
    }
    std::cerr << "MrsTransformTest: " << g_fails << " failure(s)\n";
    return 1;
}
