#include "MrsAlgebra.hxx"

#include "MrsMath.hxx"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <vector>

namespace Marshal::AnaVM {
namespace {

std::string trim(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    return s.substr(i);
}

struct Term {
    double coeff = 1.0;
    std::string rest;
};

std::vector<Term> split_plus_terms(const std::string& expr) {
    std::vector<Term> terms;
    std::string cur;
    int depth = 0;
    for (size_t i = 0; i < expr.size(); ++i) {
        const char c = expr[i];
        if (c == '(') ++depth;
        if (c == ')') --depth;
        if (depth == 0 && c == '+' && !cur.empty()) {
            terms.push_back({1.0, trim(cur)});
            cur.clear();
            continue;
        }
        if (depth == 0 && c == '-' && !cur.empty()) {
            terms.push_back({1.0, trim(cur)});
            cur = "-";
            continue;
        }
        if (depth == 0 && c == '-' && cur.empty()) {
            cur = "-";
            continue;
        }
        cur += c;
    }
    if (!trim(cur).empty()) terms.push_back({1.0, trim(cur)});
    for (auto& t : terms) {
        const std::string s = trim(t.rest);
        if (s.empty()) continue;
        if (s[0] == '-') {
            const std::string inner = trim(s.substr(1));
            try {
                size_t pos = 0;
                const double v = std::stod(inner, &pos);
                if (pos == inner.size()) {
                    t.coeff = -v;
                    t.rest = "0";
                    continue;
                }
            } catch (...) {
            }
            t.coeff = -1.0;
            t.rest = inner;
            continue;
        }
        try {
            size_t pos = 0;
            const double v = std::stod(s, &pos);
            if (pos == s.size()) {
                t.coeff = v;
                t.rest = "0";
                continue;
            }
        } catch (...) {
        }
        t.rest = s;
    }
    return terms;
}

std::string join_plus_terms(const std::vector<Term>& terms) {
    std::vector<Term> sorted = terms;
    std::sort(sorted.begin(), sorted.end(), [](const Term& a, const Term& b) {
        if (a.rest == "0" && b.rest != "0") return true;
        if (b.rest == "0" && a.rest != "0") return false;
        return a.rest < b.rest;
    });
    std::ostringstream oss;
    bool first = true;
    double const_sum = 0;
    for (const auto& t : sorted) {
        if (t.rest == "0") {
            const_sum += t.coeff;
            continue;
        }
        if (!first) oss << " + ";
        first = false;
        if (std::abs(t.coeff + 1.0) < 1e-12) oss << "-" << t.rest;
        else if (std::abs(t.coeff - 1.0) < 1e-12) oss << t.rest;
        else oss << t.coeff << " * " << t.rest;
    }
    if (std::abs(const_sum) > 1e-12) {
        if (!first) oss << " + ";
        oss << const_sum;
    }
    if (first && std::abs(const_sum) < 1e-12) return "0";
    return oss.str();
}

std::string normalize_mul_chain(std::string expr) {
    expr = trim(expr);
    if (expr == "0" || expr.find("+") != std::string::npos) return expr;
    if (expr.size() >= 3 && expr[0] == '0' && !std::isalnum(static_cast<unsigned char>(expr[2])))
        return "0";
    if (expr.size() >= 3 && expr[0] == '1' && expr[1] == '*' && std::isspace(static_cast<unsigned char>(expr[2])))
        return trim(expr.substr(2));
    return expr;
}

}  // namespace

std::string algebraic_normalize_expr(const std::string& expr, const MrsMathWitnessEnv& env) {
    const std::string t = trim(expr);
    if (t.empty()) return t;
    std::string err;
    if (const auto v = evaluate_mrs_numeric_expr(t, env, &err)) {
        std::ostringstream oss;
        oss << *v;
        return oss.str();
    }
    if (t.find('+') != std::string::npos || (t.find('-') != std::string::npos && t[0] != '-')) {
        return join_plus_terms(split_plus_terms(t));
    }
    return normalize_mul_chain(t);
}

bool algebraic_equiv_expr(const std::string& lhs, const std::string& rhs,
                          const MrsMathWitnessEnv& env, std::string* error) {
    const std::string nl = algebraic_normalize_expr(lhs, env);
    const std::string nr = algebraic_normalize_expr(rhs, env);
    if (nl == nr) return true;
    std::string err;
    const auto vl = evaluate_mrs_numeric_expr(nl, env, &err);
    const auto vr = evaluate_mrs_numeric_expr(nr, env, &err);
    if (vl && vr && std::abs(*vl - *vr) <= 1e-9) return true;
    if (error) *error = "algebraic_equiv failed: " + nl + " vs " + nr;
    return false;
}

}  // namespace Marshal::AnaVM
