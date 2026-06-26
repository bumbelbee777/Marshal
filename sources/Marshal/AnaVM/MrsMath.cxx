#include "MrsMath.hxx"

#include "MrsLadderProofGate.hxx"
#include "MrsProofTypes.hxx"
#include "Heat/GLn/GL2BSDEngine.hxx"
#include "Heat/GLn/GL2EllipseHeegnerValidation.hxx"
#include "Heat/GLn/GL2GoldbachEffectiveValidation.hxx"
#include "Heat/GLn/GL3HodgeEngine.hxx"

#include <cctype>
#include <cmath>
#include <cstring>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <vector>

// MinGW/MSVC may define math names as macros; keep builtin_* symbols and map keys stable.
#if defined(min)
#pragma push_macro("min")
#undef min
#define MRS_MATH_UNDEF_MIN 1
#endif
#if defined(max)
#pragma push_macro("max")
#undef max
#define MRS_MATH_UNDEF_MAX 1
#endif
#if defined(ceil)
#pragma push_macro("ceil")
#undef ceil
#define MRS_MATH_UNDEF_CEIL 1
#endif
#if defined(floor)
#pragma push_macro("floor")
#undef floor
#define MRS_MATH_UNDEF_FLOOR 1
#endif
#if defined(round)
#pragma push_macro("round")
#undef round
#define MRS_MATH_UNDEF_ROUND 1
#endif
#if defined(sinh)
#pragma push_macro("sinh")
#undef sinh
#define MRS_MATH_UNDEF_SINH 1
#endif
#if defined(beta)
#pragma push_macro("beta")
#undef beta
#define MRS_MATH_UNDEF_BETA 1
#endif
#if defined(log2)
#pragma push_macro("log2")
#undef log2
#define MRS_MATH_UNDEF_LOG2 1
#endif

namespace Marshal::AnaVM {
namespace {

constexpr double kEqEps = 1e-9;

std::string resolve_mrs_formal_symbol(std::string sym) {
    static const std::unordered_map<std::string, std::string> kAliases = {
        {"ξ", "xi"},           {"∏", "tprod"},        {"∀", "forall"},       {"∃", "exists"},
        {"ℝ", "reals"},        {"ℕ", "nats"},         {"ℂ", "complex"},      {"∞", "infty"},
        {"ℤ", "ints"},         {"ℚ", "rationals"},    {"∅", "empty_set"},
        {"∪", "union"},        {"∩", "intersect"},    {"⊆", "subset"},       {"∈", "in_set"},
        {"Δ", "delta"},        {"Γ", "gamma_fn"},     {"∑", "sum"},          {"∫", "integral"},
        {"∂", "partial"},
    };
    auto it = kAliases.find(sym);
    if (it != kAliases.end()) return it->second;
    if (sym.size() >= 3 && sym[0] == 'x' && sym[1] == 'i' && sym[2] == '_') return sym;
    if (sym.rfind("xi_", 0) == 0) return sym;
    if (sym == "xi") return "xi";
    return sym;
}

bool is_ident_char(unsigned char c) {
    return std::isalnum(c) || c == '_' || c >= 128;
}

std::string trim_ws(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    return s.substr(i);
}

bool evaluate_mrs_witness_expr_impl(const std::string& expr, const MrsMathWitnessEnv& env,
                                    std::string* detail, std::string* error);

std::optional<double> evaluate_mrs_numeric_expr_impl(const std::string& expr,
                                                     const MrsMathWitnessEnv& env,
                                                     std::string* error, int fn_depth = 0);

bool mrs_domain_membership(double x, const std::string& domain);

std::optional<bool> try_evaluate_mrs_formula_truth_impl(const std::string& expr,
                                                        const MrsMathWitnessEnv& env,
                                                        std::string* error);

struct Tok {
    enum Kind {
        End,
        Num,
        Ident,
        LParen,
        RParen,
        Comma,
        Plus,
        Minus,
        Star,
        Slash,
        Ge,
        Le,
        Gt,
        Lt,
        EqEq,
        NeEq,
        And,
        Or,
        Not,
        KwTrue,
        KwFalse,
        LBracket,
        RBracket,
        Impl,
        Iff,
        InSet,
    } kind = End;
    double num = 0;
    std::string ident;
};

class Lexer {
public:
    explicit Lexer(std::string src) : src_(std::move(src)) {}

    Tok next() {
        skip_ws();
        token_start_ = pos_;
        if (pos_ >= src_.size()) {
            Tok t;
            t.kind = Tok::End;
            return t;
        }
        if (match_utf8("\xe2\x9f\xb9") || match_utf8("\xe2\x87\x92")) {
            Tok t;
            t.kind = Tok::Impl;
            return t;
        }
        if (match_utf8("\xe2\x9f\xba") || match_utf8("\xe2\x87\x94")) {
            Tok t;
            t.kind = Tok::Iff;
            return t;
        }
        if (match_utf8("\xe2\x88\x88")) {
            Tok t;
            t.kind = Tok::InSet;
            return t;
        }
        if (std::isdigit(static_cast<unsigned char>(src_[pos_])) || src_[pos_] == '.') {
            const size_t start = pos_;
            while (pos_ < src_.size() &&
                   (std::isdigit(static_cast<unsigned char>(src_[pos_])) || src_[pos_] == '.' ||
                    src_[pos_] == 'e' || src_[pos_] == 'E' || src_[pos_] == '+' ||
                    src_[pos_] == '-'))
                ++pos_;
            Tok t;
            t.kind = Tok::Num;
            try {
                t.num = std::stod(src_.substr(start, pos_ - start));
            } catch (const std::exception&) {
                t.kind = Tok::End;
                if (pos_ < src_.size()) ++pos_;
            }
            return t;
        }
        if (std::isalpha(static_cast<unsigned char>(src_[pos_])) || src_[pos_] == '_' ||
            static_cast<unsigned char>(src_[pos_]) >= 128) {
            const size_t start = pos_;
            while (pos_ < src_.size() && is_ident_char(static_cast<unsigned char>(src_[pos_])))
                ++pos_;
            Tok t;
            t.ident = resolve_mrs_formal_symbol(src_.substr(start, pos_ - start));
            if (t.ident == "true") t.kind = Tok::KwTrue;
            else if (t.ident == "false") t.kind = Tok::KwFalse;
            else if (t.ident == "and") t.kind = Tok::And;
            else if (t.ident == "or") t.kind = Tok::Or;
            else if (t.ident == "not") t.kind = Tok::Not;
            else t.kind = Tok::Ident;
            return t;
        }
        if (match2("!=")) {
            Tok t;
            t.kind = Tok::NeEq;
            return t;
        }
        if (match2(">=")) {
            Tok t;
            t.kind = Tok::Ge;
            return t;
        }
        if (match2("<=")) {
            Tok t;
            t.kind = Tok::Le;
            return t;
        }
        if (match2("==")) {
            Tok t;
            t.kind = Tok::EqEq;
            return t;
        }
        if (match3("<->")) {
            Tok t;
            t.kind = Tok::Iff;
            return t;
        }
        if (match2("->")) {
            Tok t;
            t.kind = Tok::Impl;
            return t;
        }
        const char c = src_[pos_++];
        Tok t;
        switch (c) {
            case '(':
                t.kind = Tok::LParen;
                return t;
            case ')':
                t.kind = Tok::RParen;
                return t;
            case ',':
                t.kind = Tok::Comma;
                return t;
            case '+':
                t.kind = Tok::Plus;
                return t;
            case '-':
                t.kind = Tok::Minus;
                return t;
            case '*':
                t.kind = Tok::Star;
                return t;
            case '/':
                t.kind = Tok::Slash;
                return t;
            case '>':
                t.kind = Tok::Gt;
                return t;
            case '<':
                t.kind = Tok::Lt;
                return t;
            case '[':
                t.kind = Tok::LBracket;
                return t;
            case ']':
                t.kind = Tok::RBracket;
                return t;
            default:
                t.kind = Tok::End;
                return t;
        }
    }

private:
    void skip_ws() {
        while (pos_ < src_.size() && std::isspace(static_cast<unsigned char>(src_[pos_]))) ++pos_;
    }
    bool match2(const char* s) {
        if (pos_ + 1 >= src_.size()) return false;
        if (src_[pos_] == s[0] && src_[pos_ + 1] == s[1]) {
            pos_ += 2;
            return true;
        }
        return false;
    }
    bool match3(const char* s) {
        if (pos_ + 2 >= src_.size()) return false;
        if (src_[pos_] == s[0] && src_[pos_ + 1] == s[1] && src_[pos_ + 2] == s[2]) {
            pos_ += 3;
            return true;
        }
        return false;
    }
    bool match_utf8(const char* seq) {
        const size_t n = std::strlen(seq);
        if (pos_ + n > src_.size()) return false;
        if (src_.compare(pos_, n, seq) != 0) return false;
        pos_ += n;
        return true;
    }
    std::string src_;
    size_t pos_ = 0;
    size_t token_start_ = 0;

public:
    void rewind_to_current_token() { pos_ = token_start_; }

    std::string capture_until_outer_rparen() {
        skip_ws();
        const size_t start = pos_;
        int depth = 1;
        while (pos_ < src_.size()) {
            if (src_[pos_] == '(') ++depth;
            else if (src_[pos_] == ')') {
                --depth;
                if (depth == 0) {
                    const std::string pred = src_.substr(start, pos_ - start);
                    ++pos_;
                    return pred;
                }
            }
            ++pos_;
        }
        return "";
    }

    std::string capture_balanced_arg() {
        skip_ws();
        const size_t start = pos_;
        int depth = 0;
        while (pos_ < src_.size()) {
            const char c = src_[pos_];
            if (c == '(') ++depth;
            else if (c == ')') {
                if (depth == 0) {
                    std::string out = src_.substr(start, pos_ - start);
                    ++pos_;
                    return trim_ws(out);
                }
                --depth;
            } else if (c == ',' && depth == 0) {
                std::string out = src_.substr(start, pos_ - start);
                ++pos_;
                return trim_ws(out);
            }
            ++pos_;
        }
        return trim_ws(src_.substr(start));
    }

    void sync_from_string(const std::string& s) {
        src_ = s;
        pos_ = 0;
        skip_ws();
    }
};

using BuiltinFn = double (*)(const std::vector<double>&);

double builtin_ratio(const std::vector<double>& a) {
    if (a.size() != 2) return 0;
    return a[1] == 0 ? (a[0] == 0 ? 1.0 : 1e300) : a[0] / a[1];
}
double builtin_max(const std::vector<double>& a) {
    if (a.empty()) return 0;
    double m = a[0];
    for (double x : a) m = std::max(m, x);
    return m;
}
double builtin_min(const std::vector<double>& a) {
    if (a.empty()) return 0;
    double m = a[0];
    for (double x : a) m = std::min(m, x);
    return m;
}
double builtin_abs(const std::vector<double>& a) { return a.empty() ? 0 : std::abs(a[0]); }
double builtin_floor(const std::vector<double>& a) {
    return a.empty() ? 0 : std::floor(a[0]);
}
double builtin_ceil(const std::vector<double>& a) {
    return a.empty() ? 0 : std::ceil(a[0]);
}
double builtin_round(const std::vector<double>& a) {
    return a.empty() ? 0 : std::round(a[0]);
}
double builtin_sin(const std::vector<double>& a) { return a.empty() ? 0 : std::sin(a[0]); }
double builtin_cos(const std::vector<double>& a) { return a.empty() ? 0 : std::cos(a[0]); }
double builtin_tan(const std::vector<double>& a) { return a.empty() ? 0 : std::tan(a[0]); }
double builtin_exp(const std::vector<double>& a) { return a.empty() ? 0 : std::exp(a[0]); }
double builtin_log(const std::vector<double>& a) {
    return a.empty() || a[0] <= 0 ? 0 : std::log(a[0]);
}
double builtin_sqrt(const std::vector<double>& a) {
    return a.empty() || a[0] < 0 ? 0 : std::sqrt(a[0]);
}
double builtin_pow(const std::vector<double>& a) {
    if (a.size() != 2) return 0;
    return std::pow(a[0], a[1]);
}
double builtin_clamp(const std::vector<double>& a) {
    if (a.size() != 3) return 0;
    return std::max(a[1], std::min(a[0], a[2]));
}
double builtin_mod(const std::vector<double>& a) {
    if (a.size() != 2 || a[1] == 0) return 0;
    return std::fmod(a[0], a[1]);
}
double builtin_sign(const std::vector<double>& a) {
    if (a.empty()) return 0;
    return a[0] > kEqEps ? 1.0 : (a[0] < -kEqEps ? -1.0 : 0.0);
}
double builtin_hypot(const std::vector<double>& a) {
    if (a.size() != 2) return 0;
    return std::hypot(a[0], a[1]);
}
double builtin_sinh(const std::vector<double>& a) { return a.empty() ? 0 : std::sinh(a[0]); }
double builtin_cosh(const std::vector<double>& a) { return a.empty() ? 0 : std::cosh(a[0]); }
double builtin_tanh(const std::vector<double>& a) { return a.empty() ? 0 : std::tanh(a[0]); }
double builtin_asinh(const std::vector<double>& a) { return a.empty() ? 0 : std::asinh(a[0]); }
double builtin_acosh(const std::vector<double>& a) {
    return a.empty() || a[0] < 1 ? 0 : std::acosh(a[0]);
}
double builtin_atanh(const std::vector<double>& a) {
    return a.empty() || std::fabs(a[0]) >= 1 ? 0 : std::atanh(a[0]);
}
double builtin_ln(const std::vector<double>& a) { return builtin_log(a); }
double builtin_log2(const std::vector<double>& a) {
    return a.empty() || a[0] <= 0 ? 0 : std::log2(a[0]);
}
double builtin_gamma(const std::vector<double>& a) {
    if (a.empty() || a[0] <= 0) return 0;
    return std::tgamma(a[0]);
}
double builtin_lgamma(const std::vector<double>& a) {
    if (a.empty() || a[0] <= 0) return 0;
    return std::lgamma(a[0]);
}
double builtin_beta(const std::vector<double>& a) {
    if (a.size() != 2 || a[0] <= 0 || a[1] <= 0) return 0;
    return std::tgamma(a[0]) * std::tgamma(a[1]) / std::tgamma(a[0] + a[1]);
}
double builtin_erf(const std::vector<double>& a) { return a.empty() ? 0 : std::erf(a[0]); }
double builtin_erfc(const std::vector<double>& a) { return a.empty() ? 0 : std::erfc(a[0]); }

const std::vector<double>* lookup_array(const MrsMathWitnessEnv& env, const std::string& arr,
                                        std::string* err) {
    const auto it = env.arrays.find(arr);
    if (it == env.arrays.end()) {
        if (err) *err = "unknown array " + arr;
        return nullptr;
    }
    return &it->second;
}

double array_reduce(const std::vector<double>& xs, const std::string& op) {
    if (xs.empty()) return 0;
    if (op == "min") {
        double m = xs[0];
        for (double v : xs) m = std::min(m, v);
        return m;
    }
    if (op == "max") {
        double m = xs[0];
        for (double v : xs) m = std::max(m, v);
        return m;
    }
    if (op == "prod") {
        double p = 1;
        for (double v : xs) p *= v;
        return p;
    }
    double s = 0;
    for (double v : xs) s += v;
    return op == "mean" ? s / static_cast<double>(xs.size()) : s;
}

bool array_bound_cmp(const std::vector<double>& xs, double bound, const std::string& op, bool all) {
    if (xs.empty()) return false;
    auto pred = [&](double v) {
        if (op == "lt") return v < bound - kEqEps;
        if (op == "le") return v <= bound + kEqEps;
        if (op == "gt") return v > bound + kEqEps;
        if (op == "ge") return v >= bound - kEqEps;
        return false;
    };
    if (all) {
        for (double v : xs)
            if (!pred(v)) return false;
        return true;
    }
    for (double v : xs)
        if (pred(v)) return true;
    return false;
}

bool array_monotone(const std::vector<double>& xs, bool inc) {
    if (xs.size() < 2) return false;
    for (size_t i = 1; i < xs.size(); ++i) {
        if (inc && xs[i] + kEqEps < xs[i - 1]) return false;
        if (!inc && xs[i] > xs[i - 1] + kEqEps) return false;
    }
    return true;
}

double array_tail_min(const std::vector<double>& xs, double tail_count) {
    if (xs.empty()) return 0;
    const size_t k = static_cast<size_t>(std::max(1.0, tail_count));
    const size_t start = xs.size() > k ? xs.size() - k : 0;
    double m = xs[start];
    for (size_t i = start + 1; i < xs.size(); ++i) m = std::min(m, xs[i]);
    return m;
}

double array_consec_rel_max(const std::vector<double>& xs) {
    if (xs.size() < 2) return 0;
    double worst = 0;
    for (size_t i = 1; i < xs.size(); ++i) {
        const double a = xs[i - 1];
        const double b = xs[i];
        const double scale = std::max(1.0, std::max(std::fabs(a), std::fabs(b)));
        worst = std::max(worst, std::fabs(b - a) / scale);
    }
    return worst;
}

bool mrs_domain_membership(double x, const std::string& domain) {
    if (domain == "nats" || domain == "ℕ") {
        return x >= -kEqEps && std::fabs(x - std::round(x)) <= kEqEps;
    }
    if (domain == "ints" || domain == "ℤ" || domain == "integers") {
        return std::fabs(x - std::round(x)) <= kEqEps;
    }
    if (domain == "rationals" || domain == "ℚ" || domain == "rats") {
        return std::isfinite(x);
    }
    if (domain == "reals" || domain == "ℝ") {
        return std::isfinite(x);
    }
    if (domain == "complex" || domain == "ℂ") {
        return std::isfinite(x);
    }
    if (domain == "empty_set" || domain == "∅") {
        return false;
    }
    return false;
}

double builtin_implies(const std::vector<double>& a) {
    if (a.size() != 2) return 0;
    const bool p = a[0] >= 0.5 - kEqEps;
    const bool q = a[1] >= 0.5 - kEqEps;
    return (!p || q) ? 1.0 : 0.0;
}
double builtin_iff(const std::vector<double>& a) {
    if (a.size() != 2) return 0;
    const bool p = a[0] >= 0.5 - kEqEps;
    const bool q = a[1] >= 0.5 - kEqEps;
    return (p == q) ? 1.0 : 0.0;
}
double builtin_equiv(const std::vector<double>& a) {
    if (a.size() != 2) return 0;
    return std::abs(a[0] - a[1]) <= kEqEps ? 1.0 : 0.0;
}

const std::unordered_map<std::string, BuiltinFn>& builtins() {
    static const std::unordered_map<std::string, BuiltinFn> k = {
        {"ratio", builtin_ratio},
        {"max", builtin_max},
        {"min", builtin_min},
        {"abs", builtin_abs},
        {"floor", builtin_floor},
        {"ceil", builtin_ceil},
        {"round", builtin_round},
        {"sin", builtin_sin},
        {"cos", builtin_cos},
        {"tan", builtin_tan},
        {"sinh", builtin_sinh},
        {"cosh", builtin_cosh},
        {"tanh", builtin_tanh},
        {"asinh", builtin_asinh},
        {"acosh", builtin_acosh},
        {"atanh", builtin_atanh},
        {"exp", builtin_exp},
        {"log", builtin_log},
        {"ln", builtin_ln},
        {"log2", builtin_log2},
        {"sqrt", builtin_sqrt},
        {"pow", builtin_pow},
        {"clamp", builtin_clamp},
        {"mod", builtin_mod},
        {"sign", builtin_sign},
        {"hypot", builtin_hypot},
        {"gamma", builtin_gamma},
        {"lgamma", builtin_lgamma},
        {"beta", builtin_beta},
        {"erf", builtin_erf},
        {"erfc", builtin_erfc},
        {"implies", builtin_implies},
        {"implies_ok", builtin_implies},
        {"iff", builtin_iff},
        {"iff_ok", builtin_iff},
        {"equiv", builtin_equiv},
        {"equivalent", builtin_equiv},
    };
    return k;
}

bool eval_quantifier(const std::string& name, double lower, double upper,
                     const std::string& predicate_raw, const MrsMathWitnessEnv& env,
                     int max_iter, std::string* err) {
    std::string predicate = predicate_raw;
    while (!predicate.empty() && std::isspace(static_cast<unsigned char>(predicate.front())))
        predicate.erase(predicate.begin());
    while (!predicate.empty() && std::isspace(static_cast<unsigned char>(predicate.back())))
        predicate.pop_back();
    if (upper < lower) {
        if (err) *err = "quantifier empty domain";
        return false;
    }
    int steps = 0;
    if (name == "forall_even_n") {
        int start = static_cast<int>(lower);
        if (start % 2 != 0) ++start;
        for (int n = start; n <= static_cast<int>(upper); n += 2) {
            if (++steps > max_iter) {
                if (err) *err = "quantifier iteration cap exceeded";
                return false;
            }
            MrsMathWitnessEnv sub = env;
            sub.nums["n"] = static_cast<double>(n);
            std::string perr;
            if (!evaluate_mrs_witness_expr_impl(predicate, sub, nullptr, &perr)) {
                if (err)
                    *err = name + " failed at n=" + std::to_string(n) + " pred='" + predicate +
                           "' inner=" + perr;
                return false;
            }
        }
        return true;
    }
    if (name == "exists_n" || name == "forall_n") {
        const bool require_all = name == "forall_n";
        bool any = false;
        for (int n = static_cast<int>(lower); n <= static_cast<int>(upper); ++n) {
            if (++steps > max_iter) {
                if (err) *err = "quantifier iteration cap exceeded";
                return false;
            }
            MrsMathWitnessEnv sub = env;
            sub.nums["n"] = static_cast<double>(n);
            std::string perr;
            const bool ok = evaluate_mrs_witness_expr_impl(predicate, sub, nullptr, &perr);
            if (require_all && !ok) {
                if (err)
                    *err = name + " failed at n=" + std::to_string(n) + " pred='" + predicate +
                           "' inner=" + perr;
                return false;
            }
            if (!require_all && ok) any = true;
        }
        return require_all ? true : any;
    }
    if (err) *err = "unknown quantifier " + name;
    return false;
}

bool is_witness_flag_ident(const std::string& name) {
    return name.size() >= 3 &&
           (name.rfind("_ok") == name.size() - 3 || name.rfind("_cert") == name.size() - 5);
}

bool is_mrs_ident_char(unsigned char c) { return std::isalnum(c) || c == '_'; }

class Parser {
public:
    explicit Parser(std::string expr, const MrsMathWitnessEnv& env, std::string* err, int fn_depth = 0)
        : lex_(std::move(expr)), env_(env), err_(err), fn_depth_(fn_depth) {
        cur_ = lex_.next();
    }

    bool parse_bool() {
        const bool v = parse_implies();
        if (cur_.kind != Tok::End) {
            if (err_) *err_ = "trailing tokens in witness_expr";
            return false;
        }
        return v;
    }

    double parse_numeric() {
        const double v = parse_add();
        if (cur_.kind != Tok::End && err_) *err_ = "trailing tokens in numeric expr";
        return v;
    }

private:
    bool parse_implies() {
        bool v = parse_iff();
        while (cur_.kind == Tok::Impl) {
            eat(Tok::Impl);
            const bool rhs = parse_iff();
            v = !v || rhs;
        }
        return v;
    }
    bool parse_iff() {
        bool v = parse_or();
        while (cur_.kind == Tok::Iff) {
            eat(Tok::Iff);
            const bool rhs = parse_or();
            v = (v == rhs);
        }
        return v;
    }
    bool parse_or() {
        bool v = parse_and();
        while (cur_.kind == Tok::Or) {
            eat(Tok::Or);
            const bool rhs = parse_and();
            v = v || rhs;
        }
        return v;
    }
    bool parse_and() {
        bool v = parse_cmp();
        while (cur_.kind == Tok::And) {
            eat(Tok::And);
            const bool rhs = parse_cmp();
            v = v && rhs;
        }
        return v;
    }
    bool parse_cmp() {
        if (cur_.kind == Tok::Not) {
            eat(Tok::Not);
            return !parse_cmp();
        }
        if (cur_.kind == Tok::KwTrue) {
            eat(Tok::KwTrue);
            return true;
        }
        if (cur_.kind == Tok::KwFalse) {
            eat(Tok::KwFalse);
            return false;
        }
        if (cur_.kind == Tok::Ident) {
            const std::string name = cur_.ident;
            auto fit = env_.flags.find(name);
            if (fit != env_.flags.end()) {
                eat(Tok::Ident);
                const bool flag_val = fit->second;
                if (cur_.kind == Tok::Iff) {
                    eat(Tok::Iff);
                    const bool rhs = parse_iff();
                    return flag_val == rhs;
                }
                if (cur_.kind == Tok::Impl) {
                    eat(Tok::Impl);
                    const bool rhs = parse_iff();
                    return !flag_val || rhs;
                }
                return flag_val;
            }
            if (builtins().count(name) != 0) {
                const double lhs = parse_add();
                if (cur_.kind == Tok::Ge || cur_.kind == Tok::Le || cur_.kind == Tok::Gt ||
                    cur_.kind == Tok::Lt || cur_.kind == Tok::EqEq || cur_.kind == Tok::NeEq) {
                    const auto op = cur_.kind;
                    eat(op);
                    const double rhs = parse_add();
                    switch (op) {
                        case Tok::Ge:
                            return lhs >= rhs - kEqEps;
                        case Tok::Le:
                            return lhs <= rhs + kEqEps;
                        case Tok::Gt:
                            return lhs > rhs + kEqEps;
                        case Tok::Lt:
                            return lhs < rhs - kEqEps;
                        case Tok::EqEq:
                            return std::abs(lhs - rhs) <= kEqEps;
                        case Tok::NeEq:
                            return std::abs(lhs - rhs) > kEqEps;
                        default:
                            break;
                    }
                }
                return lhs != 0;
            }
            auto nit = env_.nums.find(name);
            if (nit != env_.nums.end()) {
                eat(Tok::Ident);
                if (cur_.kind == Tok::Ge || cur_.kind == Tok::Le || cur_.kind == Tok::Gt ||
                    cur_.kind == Tok::Lt || cur_.kind == Tok::EqEq || cur_.kind == Tok::NeEq) {
                    const double lhs = nit->second;
                    const auto op = cur_.kind;
                    eat(op);
                    const double rhs = parse_add();
                    switch (op) {
                        case Tok::Ge:
                            return lhs >= rhs - kEqEps;
                        case Tok::Le:
                            return lhs <= rhs + kEqEps;
                        case Tok::Gt:
                            return lhs > rhs + kEqEps;
                        case Tok::Lt:
                            return lhs < rhs - kEqEps;
                        case Tok::EqEq:
                            return std::abs(lhs - rhs) <= kEqEps;
                        case Tok::NeEq:
                            return std::abs(lhs - rhs) > kEqEps;
                        default:
                            break;
                    }
                }
                return nit->second >= 0.5 - kEqEps;
            }
        }
        const double lhs = parse_add();
        if (cur_.kind == Tok::InSet) {
            eat(Tok::InSet);
            if (cur_.kind != Tok::Ident) {
                if (err_) *err_ = "domain expected after ∈";
                return false;
            }
            const std::string domain = cur_.ident;
            eat(Tok::Ident);
            return mrs_domain_membership(lhs, domain);
        }
        if (cur_.kind == Tok::Ge || cur_.kind == Tok::Le || cur_.kind == Tok::Gt ||
            cur_.kind == Tok::Lt || cur_.kind == Tok::EqEq || cur_.kind == Tok::NeEq) {
            const auto op = cur_.kind;
            eat(op);
            const double rhs = parse_add();
            switch (op) {
                case Tok::Ge:
                    return lhs >= rhs - kEqEps;
                case Tok::Le:
                    return lhs <= rhs + kEqEps;
                case Tok::Gt:
                    return lhs > rhs + kEqEps;
                case Tok::Lt:
                    return lhs < rhs - kEqEps;
                case Tok::EqEq:
                    return std::abs(lhs - rhs) <= kEqEps;
                case Tok::NeEq:
                    return std::abs(lhs - rhs) > kEqEps;
                default:
                    break;
            }
        }
        return lhs != 0;
    }
    double parse_add() {
        double v = parse_mul();
        while (cur_.kind == Tok::Plus || cur_.kind == Tok::Minus) {
            const auto op = cur_.kind;
            eat(op);
            const double rhs = parse_mul();
            v = op == Tok::Plus ? v + rhs : v - rhs;
        }
        return v;
    }
    double parse_mul() {
        double v = parse_unary();
        while (cur_.kind == Tok::Star || cur_.kind == Tok::Slash) {
            const auto op = cur_.kind;
            eat(op);
            const double rhs = parse_unary();
            v = op == Tok::Star ? v * rhs : (rhs == 0 ? 0 : v / rhs);
        }
        return v;
    }
    double parse_unary() {
        if (cur_.kind == Tok::Minus) {
            eat(Tok::Minus);
            return -parse_unary();
        }
        return parse_primary();
    }
    double parse_primary() {
        if (cur_.kind == Tok::Num) {
            const double v = cur_.num;
            eat(Tok::Num);
            return v;
        }
        if (cur_.kind == Tok::KwTrue) {
            eat(Tok::KwTrue);
            return 1.0;
        }
        if (cur_.kind == Tok::KwFalse) {
            eat(Tok::KwFalse);
            return 0.0;
        }
        if (cur_.kind == Tok::LParen) {
            eat(Tok::LParen);
            const double v = parse_add();
            eat(Tok::RParen);
            return v;
        }
        if (cur_.kind == Tok::Ident) {
            const std::string name = cur_.ident;
            eat(Tok::Ident);
            if (cur_.kind == Tok::LParen) {
                eat(Tok::LParen);
                if (name == "in_set" || name == "member") {
                    const double x = parse_add();
                    eat(Tok::Comma);
                    if (cur_.kind != Tok::Ident) {
                        if (err_) *err_ = "in_set domain must be a set symbol";
                        return 0;
                    }
                    const std::string domain = cur_.ident;
                    eat(Tok::Ident);
                    eat(Tok::RParen);
                    return mrs_domain_membership(x, domain) ? 1.0 : 0.0;
                }
                const auto fn_it = env_.fns.find(name);
                if (fn_it != env_.fns.end()) {
                    std::vector<double> args;
                    if (cur_.kind != Tok::RParen) {
                        args.push_back(parse_add());
                        while (cur_.kind == Tok::Comma) {
                            eat(Tok::Comma);
                            args.push_back(parse_add());
                        }
                    }
                    eat(Tok::RParen);
                    if (args.size() != fn_it->second.params.size()) {
                        if (err_) *err_ = "fn arity mismatch for " + name;
                        return 0;
                    }
                    if (fn_depth_ > 8) {
                        if (err_) *err_ = "fn call depth exceeded";
                        return 0;
                    }
                    MrsMathWitnessEnv sub = env_;
                    for (size_t i = 0; i < args.size(); ++i)
                        sub.nums[fn_it->second.params[i]] = args[i];
                    std::string nerr;
                    const auto val =
                        evaluate_mrs_numeric_expr_impl(fn_it->second.body, sub, &nerr, fn_depth_ + 1);
                    if (!val.has_value()) {
                        if (err_) *err_ = nerr.empty() ? "fn body eval failed" : nerr;
                        return 0;
                    }
                    return *val;
                }
                if (name == "len" && cur_.kind == Tok::Ident) {
                    const std::string arr = cur_.ident;
                    eat(Tok::Ident);
                    eat(Tok::RParen);
                    const auto it = env_.arrays.find(arr);
                    if (it == env_.arrays.end()) {
                        if (err_) *err_ = "unknown array " + arr;
                        return 0;
                    }
                    return static_cast<double>(it->second.size());
                }
                if (name == "sum" && cur_.kind == Tok::Ident) {
                    const std::string arr = cur_.ident;
                    eat(Tok::Ident);
                    eat(Tok::RParen);
                    const auto* xs = lookup_array(env_, arr, err_);
                    if (!xs) return 0;
                    return array_reduce(*xs, "sum");
                }
                if ((name == "prod" || name == "mean" || name == "arr_min" || name == "arr_max") &&
                    cur_.kind == Tok::Ident) {
                    const std::string arr = cur_.ident;
                    eat(Tok::Ident);
                    eat(Tok::RParen);
                    const auto* xs = lookup_array(env_, arr, err_);
                    if (!xs) return 0;
                    const std::string op =
                        name == "prod" ? "prod"
                                       : (name == "mean" ? "mean"
                                                         : (name == "arr_min" ? "min" : "max"));
                    return array_reduce(*xs, op);
                }
                if ((name == "arr_all_lt" || name == "arr_all_le" || name == "arr_all_gt" ||
                     name == "arr_all_ge" || name == "arr_any_lt" || name == "arr_any_gt") &&
                    cur_.kind == Tok::Ident) {
                    const std::string arr = cur_.ident;
                    eat(Tok::Ident);
                    eat(Tok::Comma);
                    const double bound = parse_add();
                    eat(Tok::RParen);
                    const auto* xs = lookup_array(env_, arr, err_);
                    if (!xs) return 0;
                    const bool all = name.rfind("arr_all_", 0) == 0;
                    std::string op = "lt";
                    if (name == "arr_all_le" || name == "arr_any_le") op = "le";
                    else if (name == "arr_all_gt" || name == "arr_any_gt") op = "gt";
                    else if (name == "arr_all_ge" || name == "arr_any_ge") op = "ge";
                    return array_bound_cmp(*xs, bound, op, all) ? 1.0 : 0.0;
                }
                if ((name == "arr_monotone_inc" || name == "arr_monotone_dec") &&
                    cur_.kind == Tok::Ident) {
                    const std::string arr = cur_.ident;
                    eat(Tok::Ident);
                    eat(Tok::RParen);
                    const auto* xs = lookup_array(env_, arr, err_);
                    if (!xs) return 0;
                    return array_monotone(*xs, name == "arr_monotone_inc") ? 1.0 : 0.0;
                }
                if (name == "arr_tail_min" && cur_.kind == Tok::Ident) {
                    const std::string arr = cur_.ident;
                    eat(Tok::Ident);
                    eat(Tok::Comma);
                    const double tail = parse_add();
                    eat(Tok::RParen);
                    const auto* xs = lookup_array(env_, arr, err_);
                    if (!xs) return 0;
                    return array_tail_min(*xs, tail);
                }
                if (name == "arr_consec_rel_max" && cur_.kind == Tok::Ident) {
                    const std::string arr = cur_.ident;
                    eat(Tok::Ident);
                    eat(Tok::RParen);
                    const auto* xs = lookup_array(env_, arr, err_);
                    if (!xs) return 0;
                    return array_consec_rel_max(*xs);
                }
                if (name == "bound_one_sided") {
                    const double x = parse_add();
                    eat(Tok::Comma);
                    const double lb = parse_add();
                    eat(Tok::RParen);
                    return x >= lb - kEqEps ? 1.0 : 0.0;
                }
                if (name == "bound_two_sided") {
                    const double x = parse_add();
                    eat(Tok::Comma);
                    const double lb = parse_add();
                    eat(Tok::Comma);
                    const double ub = parse_add();
                    eat(Tok::RParen);
                    return (x >= lb - kEqEps && x <= ub + kEqEps) ? 1.0 : 0.0;
                }
                if (name == "arith_eq" || name == "algebraic_eq") {
                    const double left = parse_add();
                    eat(Tok::Comma);
                    const double right = parse_add();
                    eat(Tok::RParen);
                    return std::abs(left - right) <= kEqEps ? 1.0 : 0.0;
                }
                if (name == "arith_norm" || name == "algebraic_norm") {
                    const double v = parse_add();
                    eat(Tok::RParen);
                    return v;
                }
                if (name == "forall_even_n" || name == "exists_n" || name == "forall_n" ||
                    name == "forall" || name == "exists") {
                    std::string qname = name;
                    if (qname == "forall") qname = "forall_n";
                    if (qname == "exists") qname = "exists_n";
                    const double lower = parse_add();
                    eat(Tok::Comma);
                    const double upper = parse_add();
                    eat(Tok::Comma);
                    lex_.rewind_to_current_token();
                    const std::string predicate = lex_.capture_until_outer_rparen();
                    int max_iter = 10000;
                    if (auto it = env_.nums.find("goldbach_effective_n_max");
                        it != env_.nums.end())
                        max_iter = static_cast<int>(it->second);
                    std::string qerr;
                    const bool ok =
                        eval_quantifier(qname, lower, upper, predicate, env_, max_iter, &qerr);
                    if (!ok && err_) *err_ = qerr;
                    cur_ = lex_.next();
                    return ok ? 1.0 : 0.0;
                }
                std::vector<double> args;
                if (cur_.kind != Tok::RParen) {
                    args.push_back(parse_add());
                    while (cur_.kind == Tok::Comma) {
                        eat(Tok::Comma);
                        args.push_back(parse_add());
                    }
                }
                eat(Tok::RParen);
                const auto& bi = builtins();
                const auto it = bi.find(name);
                if (it == bi.end()) {
                    if (err_) *err_ = "unknown builtin " + name;
                    return 0;
                }
                return it->second(args);
            }
            if (cur_.kind == Tok::LBracket) {
                eat(Tok::LBracket);
                const int idx = static_cast<int>(parse_add());
                eat(Tok::RBracket);
                const auto it = env_.arrays.find(name);
                if (it == env_.arrays.end()) {
                    if (err_) *err_ = "unknown array " + name;
                    return 0;
                }
                if (idx < 0 || static_cast<size_t>(idx) >= it->second.size()) {
                    if (err_) *err_ = "array index out of range";
                    return 0;
                }
                return it->second[static_cast<size_t>(idx)];
            }
            const auto nit = env_.nums.find(name);
            if (nit != env_.nums.end()) return nit->second;
            const auto fit = env_.flags.find(name);
            if (fit != env_.flags.end()) return fit->second ? 1.0 : 0.0;
            if (err_) *err_ = "unknown identifier " + name;
            return 0;
        }
        if (err_) *err_ = "unexpected token in witness_expr";
        return 0;
    }
    void eat(Tok::Kind k) {
        if (cur_.kind != k) {
            if (err_) *err_ = "parse error in witness_expr";
            return;
        }
        cur_ = lex_.next();
    }

    Lexer lex_;
    const MrsMathWitnessEnv& env_;
    std::string* err_;
    int fn_depth_ = 0;
    Tok cur_;
};

void put_num(MrsMathWitnessEnv* env, const char* k, double v) {
    if (env) env->nums[k] = v;
}
void put_flag(MrsMathWitnessEnv* env, const char* k, bool v) {
    if (env) env->flags[k] = v;
}

std::optional<double> evaluate_mrs_numeric_expr_impl(const std::string& expr,
                                                     const MrsMathWitnessEnv& env,
                                                     std::string* error, int fn_depth) {
    if (expr.empty()) return std::nullopt;
    std::string err;
    Parser p(expr, env, &err, fn_depth);
    const double v = p.parse_numeric();
    if (!err.empty()) {
        if (error) *error = err;
        return std::nullopt;
    }
    return v;
}

bool evaluate_mrs_witness_expr_impl(const std::string& expr, const MrsMathWitnessEnv& env,
                                    std::string* detail, std::string* error) {
    if (expr.empty()) {
        if (error) *error = "empty witness_expr";
        return false;
    }
    std::string err;
    Parser p(expr, env, &err);
    const bool ok = p.parse_bool();
    if (!err.empty()) {
        if (error) *error = err;
        return false;
    }
    if (detail) *detail = std::string("mrs_math:") + expr + " => " + (ok ? "true" : "false");
    return ok;
}

std::optional<bool> try_evaluate_mrs_formula_truth_impl(const std::string& expr,
                                                        const MrsMathWitnessEnv& env,
                                                        std::string* error) {
    if (expr.empty()) return std::nullopt;
    const std::string norm = normalize_mrs_formula_unicode(expr);
    if (norm.find("—") != std::string::npos && norm.find('_') == std::string::npos &&
        norm.find(" and ") == std::string::npos)
        return std::nullopt;
    std::string err;
    Parser p(norm, env, &err);
    const bool v = p.parse_bool();
    if (!err.empty()) return std::nullopt;
    if (error) *error = err;
    return v;
}

}  // namespace

std::string normalize_mrs_formula_unicode(std::string s) {
    auto rep = [&](const char* from, const char* to) {
        size_t pos = 0;
        const std::string needle(from);
        const std::string repl(to);
        while ((pos = s.find(needle, pos)) != std::string::npos) {
            s.replace(pos, needle.size(), repl);
            pos += repl.size();
        }
    };
    rep("⟹", " -> ");
    rep("⟺", " <-> ");
    rep("→", " -> ");
    rep("↔", " <-> ");
    rep("⊢", " implies ");
    rep("∧", " and ");
    rep("∨", " or ");
    rep("¬", " not ");
    rep("∀", "forall");
    rep("∃", "exists");
    return trim_ws(s);
}

namespace {

bool mrs_line_has_formal_token(const std::string& line) {
    if (line.find('_') != std::string::npos) return true;
    if (line.find('<') != std::string::npos || line.find('>') != std::string::npos) return true;
    if (line.find(" and ") != std::string::npos || line.find(" or ") != std::string::npos) return true;
    if (line.find("forall") != std::string::npos || line.find("exists") != std::string::npos)
        return true;
    if (line.find("implies") != std::string::npos || line.find("iff") != std::string::npos)
        return true;
    if (line.find("->") != std::string::npos || line.find("<->") != std::string::npos) return true;
    return false;
}

bool mrs_line_is_prose_only(const std::string& line) {
    if (line.empty()) return true;
    if (line.rfind("classical ", 0) == 0) return true;
    if (line.rfind("(proof:", 0) == 0 || line.rfind("(PROVED", 0) == 0) return true;
    if (line.find("(proof:") != std::string::npos && !mrs_line_has_formal_token(line)) return true;
    if (line.find("—") != std::string::npos && !mrs_line_has_formal_token(line)) return true;
    if (line.find("scope is") != std::string::npos) return true;
    return false;
}

std::string take_after_turnstile(std::string line) {
    const size_t t = line.find("⊢");
    if (t != std::string::npos) {
        line = trim_ws(line.substr(t + 3));
    }
    const size_t imp = line.find(" implies ");
    if (imp != std::string::npos) {
        line = trim_ws(line.substr(imp + 9));
    }
    return line;
}

}  // namespace

std::string extract_formal_mrs_conclusion(const std::string& raw) {
    std::vector<std::string> parts;
    std::stringstream ss(raw);
    std::string line;
    while (std::getline(ss, line)) {
        line = trim_ws(line);
        if (line.empty()) continue;
        if (line.rfind("check:", 0) == 0) line = trim_ws(line.substr(6));
        line = take_after_turnstile(std::move(line));
        if (mrs_line_is_prose_only(line)) continue;
        line = normalize_mrs_formula_unicode(line);
        if (mrs_line_has_formal_token(line)) parts.push_back(line);
    }
    if (parts.empty()) return normalize_mrs_formula_unicode(trim_ws(raw));
    std::string out = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) out += " and " + parts[i];
    return out;
}

void merge_mrs_arrays_into_env(const MrsCompilationBundle& bundle, MrsMathWitnessEnv* env) {
    if (!env) return;
    for (const auto& m : bundle.merged_modules) {
        for (const auto& a : m.arrays) {
            if (!a.name.empty()) env->arrays[a.name] = a.elements;
        }
    }
}

void merge_mrs_fns_into_env(const MrsCompilationBundle& bundle, MrsMathWitnessEnv* env) {
    if (!env) return;
    for (const auto& m : bundle.merged_modules) {
        for (const auto& f : m.fns) {
            if (!f.name.empty()) env->fns[f.name] = MrsFnRecord{f.params, f.body};
        }
    }
}

MrsMathWitnessEnv build_mrs_math_witness_env(const MrsLadderWitnessContext& ctx,
                                               const Config& cfg) {
    MrsMathWitnessEnv env;
    const auto& b = cfg.bound_audit;
    put_num(&env, "grid_rel_gap_ub", b.grid_rel_gap_ub);
    put_num(&env, "grid_mult_dev_ub", b.grid_mult_dev_ub);
    put_num(&env, "tail_bound_decades_ub", b.tail_bound_decades_ub);
    put_num(&env, "ident_gap_decades_ub", b.ident_gap_decades_ub);
    put_num(&env, "log_partial_sum_ub", b.log_partial_sum_ub);
    put_num(&env, "l_function_grid_rel_gap_ub", b.l_function_grid_rel_gap_ub);
    put_num(&env, "sha_resolvent_gap_ub", b.sha_resolvent_gap_ub);
    put_num(&env, "holomorphy_uniform_gap_ub", b.holomorphy_uniform_gap_ub);
    put_num(&env, "kernel_tolerance", b.kernel_tolerance);
    put_num(&env, "hodge_h11_target", b.hodge_h11_target);
    put_num(&env, "major_arc_threshold", b.major_arc_threshold);
    put_num(&env, "minor_arc_ub", b.minor_arc_ub);
    put_num(&env, "goldbach_n0", b.goldbach_n0);
    put_num(&env, "goldbach_effective_n_max", b.goldbach_effective_n_max);
    put_num(&env, "goldbach_extension_ratio_lb", b.goldbach_extension_ratio_lb);
    put_num(&env, "bsd_extension_ratio_lb", b.bsd_extension_ratio_lb);
    put_num(&env, "bsd_formula_rel_gap_ub", b.bsd_formula_rel_gap_ub);
    put_num(&env, "bsd_millennium_extension_ratio_lb", b.bsd_millennium_extension_ratio_lb);
    put_num(&env, "hodge_extension_ratio_lb", b.hodge_extension_ratio_lb);
    put_num(&env, "hodge_millennium_extension_ratio_lb", b.hodge_millennium_extension_ratio_lb);
    put_num(&env, "hodge_millennium_pp_target", b.hodge_millennium_pp_target);
    put_num(&env, "rooted_rmse_ub", b.rooted_rmse_ub);
    put_num(&env, "gauge_over_gravity_lb", b.gauge_over_gravity_lb);
    put_num(&env, "holy_anchor_t", b.holy_anchor_t);
    put_num(&env, "ym_mass_gap_lb", b.ym_mass_gap_lb);
    put_num(&env, "ym_extension_ratio_lb", b.ym_extension_ratio_lb);
    put_num(&env, "ym_millennium_extension_ratio_lb", b.ym_millennium_extension_ratio_lb);
    put_num(&env, "holy_stationarity_residual_ub", b.holy_stationarity_residual_ub);
    put_num(&env, "ym_lattice_beta", b.ym_lattice_beta);
    put_num(&env, "ym_lattice_volume", b.ym_lattice_volume);
    put_flag(&env, "rh_capstone_ok", ctx.rh_capstone_ok);
    put_flag(&env, "bsd_capstone_ok", ctx.bsd_capstone_ok);
    put_flag(&env, "hodge_capstone_ok", ctx.hodge_capstone_ok);

    if (ctx.bsd) {
        put_num(&env, "l_function_grid_rel_gap", static_cast<double>(ctx.bsd->l_function_grid_rel_gap));
        put_num(&env, "sha_resolvent_gap", static_cast<double>(ctx.bsd->sha_resolvent_gap));
        put_num(&env, "holomorphy_uniform_gap", static_cast<double>(ctx.bsd->holomorphy_uniform_gap));
        put_num(&env, "algebraic_rank", ctx.bsd->algebraic_rank);
        put_num(&env, "kernel_multiplicity", ctx.bsd->kernel_multiplicity);
        put_flag(&env, "l_grid_ok", ctx.bsd->l_grid_ok);
        put_flag(&env, "holomorphy_ok", ctx.bsd->holomorphy_ok);
        put_flag(&env, "rank_match_ok", ctx.bsd->rank_match_ok);
        put_flag(&env, "sha_gap_ok", ctx.bsd->sha_gap_ok);
        put_flag(&env, "regulator_ok", ctx.bsd->regulator_ok);
        put_flag(&env, "tamagawa_ok", ctx.bsd->tamagawa_ok);
        put_flag(&env, "sha_order_ok", ctx.bsd->sha_order_ok);
        put_flag(&env, "omega_ok", ctx.bsd->omega_ok);
        put_flag(&env, "bsd_formula_ok", ctx.bsd->bsd_formula_ok);
        put_num(&env, "regulator_spectral", static_cast<double>(ctx.bsd->regulator_spectral));
        put_num(&env, "tamagawa_product", static_cast<double>(ctx.bsd->tamagawa_product));
        put_num(&env, "sha_order_cert", ctx.bsd->sha_order_cert);
        put_num(&env, "omega_period", static_cast<double>(ctx.bsd->omega_period));
        put_num(&env, "leading_coeff_lhs", static_cast<double>(ctx.bsd->leading_coeff_lhs));
        put_num(&env, "leading_coeff_rhs", static_cast<double>(ctx.bsd->leading_coeff_rhs));
        put_num(&env, "bsd_formula_rel_gap", static_cast<double>(ctx.bsd->bsd_formula_rel_gap));
        put_num(&env, "bsd_formula_margin_ratio",
                static_cast<double>(ctx.bsd->bsd_formula_margin_ratio));
        put_flag(&env, "bsd_rank_proved", ctx.bsd->bsd_rank_proved);
        put_flag(&env, "bsd_millennium_proved", ctx.bsd->bsd_millennium_proved);
    }
    if (ctx.hodge) {
        put_num(&env, "predicted_hodge_multiplicity", ctx.hodge->predicted_hodge_multiplicity);
        put_num(&env, "hodge_kernel_multiplicity", ctx.hodge->kernel_multiplicity);
        put_flag(&env, "hodge_match_ok", ctx.hodge->hodge_match_ok);
        put_flag(&env, "cycle_map_ok", ctx.hodge->cycle_map_ok);
        put_flag(&env, "cycle_constructive_ok", ctx.hodge->cycle_constructive_ok);
        put_flag(&env, "hodge_pp_ok", ctx.hodge->hodge_pp_ok);
        put_flag(&env, "hodge_millennium_ok", ctx.hodge->hodge_millennium_ok);
        put_num(&env, "hitchin_divisor_count", ctx.hodge->hitchin_divisor_count);
        put_num(&env, "cycle_constructive_span", ctx.hodge->cycle_constructive_span);
        put_num(&env, "hodge_pp_2_0", ctx.hodge->hodge_pp_2_0);
        put_num(&env, "hodge_pp_1_1", ctx.hodge->hodge_pp_1_1);
        put_num(&env, "hodge_pp_0_2", ctx.hodge->hodge_pp_0_2);
        put_num(&env, "hodge_millennium_pp_match", ctx.hodge->hodge_millennium_pp_match);
        if (!env.nums.count("hodge_millennium_pp_target"))
            put_num(&env, "hodge_millennium_pp_target", ctx.hodge->hodge_millennium_pp_target);
        put_flag(&env, "rank3_contract_ok", ctx.hodge->rank3_contract_ok);
        put_flag(&env, "hodge_conjecture_proved", ctx.hodge->hodge_conjecture_proved);
        put_flag(&env, "hodge_millennium_proved", ctx.hodge->hodge_millennium_proved);
    }
    if (ctx.goldbach) {
        put_num(&env, "major_arc_spectral_mass",
                static_cast<double>(ctx.goldbach->major_arc_spectral_mass));
        put_num(&env, "minor_arc_bound", static_cast<double>(ctx.goldbach->minor_arc_bound));
        put_flag(&env, "major_arc_ok", ctx.goldbach->major_arc_ok);
        put_flag(&env, "minor_arc_ok", ctx.goldbach->minor_arc_ok);
        put_flag(&env, "goldbach_shared_gln2_ok", ctx.goldbach->goldbach_shared_gln2_ok);
    }
    if (ctx.goldbach_effective) {
        put_flag(&env, "effective_ok", ctx.goldbach_effective->ok);
        put_num(&env, "goldbach_even_count", ctx.goldbach_effective->even_count);
        put_num(&env, "goldbach_n_max_checked", ctx.goldbach_effective->n_max_checked);
    }
    if (ctx.ym) {
        put_num(&env, "gauge_smallest_positive_eigenvalue",
                static_cast<double>(ctx.ym->gauge_smallest_positive_eigenvalue));
        put_num(&env, "lattice_gap_estimate", static_cast<double>(ctx.ym->lattice_gap_estimate));
        put_flag(&env, "rank4_contract_ok", ctx.ym->rank4_contract_ok);
        put_flag(&env, "self_adjoint_ok", ctx.ym->self_adjoint_ok);
        put_flag(&env, "os_axioms_ok", ctx.ym->os_axioms_ok);
        put_flag(&env, "mass_gap_ok", ctx.ym->mass_gap_ok);
        put_flag(&env, "lattice_gap_ok", ctx.ym->lattice_gap_ok);
        put_flag(&env, "su3_reduction_ok", ctx.ym->su3_reduction_ok);
        put_num(&env, "gauge_over_gravity", static_cast<double>(ctx.ym->gauge_over_gravity));
        put_num(&env, "rooted_blended_rmse", static_cast<double>(ctx.ym->rooted_blended_rmse));
        put_flag(&env, "theta_stable", ctx.ym->theta_stable);
        put_flag(&env, "outlook_ok", ctx.ym->outlook_ok);
        put_flag(&env, "os_reflection_ok", ctx.ym->os_reflection_ok);
        put_num(&env, "os_reflection_residual", static_cast<double>(ctx.ym->os_reflection_residual));
        put_flag(&env, "ym_mass_gap_proved", ctx.ym->ym_mass_gap_proved);
        if (env.nums.count("ym_mass_gap_lb") && env.nums.at("ym_mass_gap_lb") > 0 &&
            env.nums.count("gauge_smallest_positive_eigenvalue"))
            put_num(&env, "ym_gap_extension_ratio",
                    env.nums.at("gauge_smallest_positive_eigenvalue") /
                        env.nums.at("ym_mass_gap_lb"));
    }

    const double minor =
        env.nums.count("minor_arc_bound") ? std::max(env.nums["minor_arc_bound"], 1e-12) : 1e-12;
    if (env.nums.count("major_arc_spectral_mass"))
        put_num(&env, "major_minor_ratio", env.nums["major_arc_spectral_mass"] / minor);
    const double l_gap = env.nums.count("l_function_grid_rel_gap")
                             ? std::max(env.nums["l_function_grid_rel_gap"], 1e-12)
                             : 1e-12;
    if (env.nums.count("l_function_grid_rel_gap_ub"))
        put_num(&env, "l_grid_margin_ratio", env.nums["l_function_grid_rel_gap_ub"] / l_gap);

    return env;
}

bool evaluate_mrs_witness_expr(const std::string& expr, const MrsMathWitnessEnv& env,
                               std::string* detail, std::string* error) {
    MrsMathWitnessEnv seeded = env;
    seed_missing_witness_flags(expr, &seeded);
    return evaluate_mrs_witness_expr_impl(expr, seeded, detail, error);
}

std::optional<double> evaluate_mrs_numeric_expr(const std::string& expr,
                                                const MrsMathWitnessEnv& env,
                                                std::string* error) {
    MrsMathWitnessEnv seeded = env;
    seed_missing_witness_flags(expr, &seeded);
    return evaluate_mrs_numeric_expr_impl(expr, seeded, error, 0);
}

std::optional<bool> try_evaluate_mrs_formula_truth(const std::string& expr,
                                                   const MrsMathWitnessEnv& env,
                                                   std::string* error) {
    MrsMathWitnessEnv seeded = env;
    seed_missing_witness_flags(expr, &seeded);
    return try_evaluate_mrs_formula_truth_impl(expr, seeded, error);
}

void seed_missing_witness_flags(const std::string& expr, MrsMathWitnessEnv* env) {
    if (!env) return;
    for (size_t i = 0; i < expr.size();) {
        while (i < expr.size() && !std::isalpha(static_cast<unsigned char>(expr[i])) &&
               expr[i] != '_')
            ++i;
        if (i >= expr.size()) break;
        const size_t start = i;
        while (i < expr.size() && is_mrs_ident_char(static_cast<unsigned char>(expr[i]))) ++i;
        const std::string tok = expr.substr(start, i - start);
        if (tok == "and" || tok == "or" || tok == "not" || tok == "true" || tok == "false" ||
            tok == "implies" || tok == "iff" || tok == "equiv" || tok == "in_set" || tok == "member" ||
            tok == "arith_eq" || tok == "algebraic_eq" || tok == "arith_norm" || tok == "algebraic_norm")
            continue;
        if (builtins().count(tok) != 0) continue;
        if (!is_witness_flag_ident(tok)) continue;
        if (!env->flags.count(tok) && !env->nums.count(tok)) env->nums[tok] = 0.0;
    }
}

bool mrs_math_has_builtin(const std::string& name) {
    if (builtins().count(name) != 0) return true;
    return name == "in_set" || name == "member" || name == "len" || name == "sum" ||
           name == "arith_eq" || name == "algebraic_eq" || name == "arith_norm" ||
           name == "algebraic_norm" ||
           name.rfind("arr_", 0) == 0 || name.rfind("forall_", 0) == 0 || name == "exists_n" ||
           name.rfind("bound_", 0) == 0;
}

}  // namespace Marshal::AnaVM
