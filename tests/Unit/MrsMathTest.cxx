// MrsMath witness_expr unit tests.

#include "AnaVM/MrsMath.hxx"

#include <iostream>

namespace {

int g_fails = 0;

void check(const char* expr, Marshal::AnaVM::MrsMathWitnessEnv& env) {
    std::string err;
    if (!Marshal::AnaVM::evaluate_mrs_witness_expr(expr, env, nullptr, &err)) {
        std::cerr << "FAIL: " << expr;
        if (!err.empty()) std::cerr << " (" << err << ")";
        std::cerr << "\n";
        ++g_fails;
    }
}

}  // namespace

int main() {
    if (!Marshal::AnaVM::mrs_math_has_builtin("ceil")) {
        std::cerr << "FAIL: ceil builtin not registered\n";
        return 1;
    }
    Marshal::AnaVM::MrsMathWitnessEnv env;
    env.nums["x"] = 3.5;
    env.nums["goldbach_effective_n_max"] = 100;
    env.flags["holomorphy_ok"] = true;
    env.flags["grid_pin_ok"] = true;
    env.fns["double"] = Marshal::AnaVM::MrsFnRecord{{"x"}, "x + x"};
    env.fns["halve"] = Marshal::AnaVM::MrsFnRecord{{"x"}, "x / 2"};

    check("floor(x) == 3", env);
    check("3 != 4", env);
    {
        Marshal::AnaVM::MrsMathWitnessEnv sub = env;
        sub.nums["n"] = 4;
        check("n >= 4", sub);
    }
    check("forall_n(4, 10, n >= 4)", env);
    check("exists_n(1, 5, n == 3)", env);
    check("log(exp(1)) > 0.9", env);
    check("ln(exp(1)) > 0.99", env);
    check("erf(1) > 0.8", env);
    check("gamma(2) >= 0.99", env);
    check("cos(0) == 1", env);
    check("sin(0) == 0", env);
    check("round(x) >= 3.5", env);
    check("ceil(3.5) >= 3.5", env);
    check("ceil(x) >= 3.5", env);
    check("log2(8) == 3", env);
    check("sinh(0) == 0", env);
    check("beta(2,2) >= 0.16", env);

    env.arrays["SEQ_DEC"] = {10.0, 8.0, 6.0, 4.0};
    env.arrays["SEQ_INC"] = {1.0, 2.0, 3.0};
    env.arrays["MARGIN"] = {100.0, 200.0, 50.0};
    check("arr_monotone_dec(SEQ_DEC)", env);
    check("arr_monotone_inc(SEQ_INC)", env);
    check("arr_all_ge(MARGIN, 0)", env);
    check("arr_consec_rel_max(SEQ_DEC) <= 0.34", env);
    check("bound_one_sided(5, 0)", env);

    check("implies(holomorphy_ok, holomorphy_ok)", env);
    check("holomorphy_ok -> holomorphy_ok", env);
    check("iff_ok(true, true)", env);
    check("grid_pin_ok <-> grid_pin_ok", env);
    check("equiv(1, 1)", env);
    check("in_set(4, nats)", env);
    check("4 ∈ ints", env);
    check("double(3) == 6", env);
    check("halve(8) == 4", env);
    check("arith_eq(ratio(4,2), 2)", env);
    check("arith_eq(2, 2)", env);
    check("arith_norm(2) == 2", env);

    if (g_fails == 0) {
        std::cout << "MrsMathTest: all passed\n";
        return 0;
    }
    std::cerr << "MrsMathTest: " << g_fails << " failure(s)\n";
    return 1;
}
