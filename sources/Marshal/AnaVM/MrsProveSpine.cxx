#include "MrsProveSpine.hxx"

#include "MrsProofLogic.hxx"

#include <algorithm>

#include <cctype>

#include <functional>

#include <unordered_map>

#include <unordered_set>



namespace Marshal::AnaVM {

namespace {



std::string trim(const std::string& s) {

    size_t a = 0;

    while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) ++a;

    size_t b = s.size();

    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;

    return s.substr(a, b - a);

}



bool is_ident_char(char c) {

    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';

}



bool is_single_identifier_body(const std::string& body) {

    const std::string t = trim(body);

    if (t.empty() || t.find('(') != std::string::npos) return false;

    if (!std::isalpha(static_cast<unsigned char>(t[0])) && t[0] != '_') return false;

    for (char c : t) {

        if (!is_ident_char(c)) return false;

    }

    return true;

}



std::unordered_map<std::string, std::string> collect_prove_bodies(

    const MrsCompilationBundle& bundle) {

    std::unordered_map<std::string, std::string> out;

    for (const auto& m : bundle.merged_modules) {

        for (const auto& p : m.proves) {

            if (!p.name.empty()) out[p.name] = p.body;

        }

    }

    return out;

}



std::unordered_set<std::string> ladder_obligation_prove_refs(

    const MrsCompilationBundle& bundle) {

    std::unordered_set<std::string> refs;

    static const std::unordered_set<std::string> kLadderGraphs = {

        "MarshalBSD", "MarshalBSDMillennium", "MarshalBSDUniversal", "MarshalHodge",
        "MarshalHodgeMillennium", "MarshalHodgeUniversal", "MarshalYM", "MarshalYMMillennium",
        "MarshalYMUniversal", "MarshalGoldbach"};

    for (const auto& m : bundle.merged_modules) {

        for (const auto& g : m.proof_graphs) {

            if (!kLadderGraphs.count(g.name)) continue;

            for (const auto& ob : g.obligations) {

                if (ob.proof_class == ProofClass::Analytic ||
                    ob.proof_class == ProofClass::ClassicalImport ||
                    ob.proof_class == ProofClass::Reduction ||
                    ob.proof_class == ProofClass::AnalyticOpen ||

                    ob.proof_class == ProofClass::Composition ||

                    ob.proof_class == ProofClass::Universal ||

                    ob.proof_class == ProofClass::Inductive ||

                    ob.proof_class == ProofClass::Convergent ||

                    ob.proof_class == ProofClass::Numeric ||
                    ob.proof_class == ProofClass::NumericInterval) {

                    if (!ob.prove_ref.empty()) refs.insert(ob.prove_ref);

                    if (!ob.extend_via.empty()) refs.insert(ob.extend_via);

                }

            }

        }

    }

    return refs;

}



}  // namespace



std::vector<std::string> prove_body_callees(const std::string& body) {

    std::vector<std::string> out;

    const std::string t = trim(body);

    if (t.empty()) return out;



    const auto open = t.find('(');

    if (open == std::string::npos) {

        if (is_single_identifier_body(t)) out.push_back(t);

        return out;

    }



    int depth = 0;

    std::string token;

    auto flush = [&]() {

        const std::string id = trim(token);

        token.clear();

        if (!id.empty() && (std::isalpha(static_cast<unsigned char>(id[0])) || id[0] == '_'))

            out.push_back(id);

    };



    for (size_t i = open + 1; i < t.size(); ++i) {

        const char c = t[i];

        if (c == '(') {

            flush();

            ++depth;

            continue;

        }

        if (c == ')') {

            flush();

            if (depth == 0) break;

            --depth;

            continue;

        }

        if (depth == 0 && c == ',') {

            flush();

            continue;

        }

        if (depth == 0 && !std::isspace(static_cast<unsigned char>(c))) token += c;

    }

    flush();

    return out;

}



MrsProveSpineReport validate_ladder_prove_spine(const MrsCompilationBundle& bundle) {

    MrsProveSpineReport rep;

    const auto bodies = collect_prove_bodies(bundle);

    const auto obligation_refs = ladder_obligation_prove_refs(bundle);



    std::unordered_set<std::string> prove_names;

    for (const auto& m : bundle.merged_modules) {

        for (const auto& p : m.proves) {

            if (!p.name.empty()) prove_names.insert(p.name);

        }

    }



    static const std::unordered_set<std::string> kLadderGraphs = {

        "MarshalBSD", "MarshalBSDMillennium", "MarshalBSDUniversal", "MarshalHodge",
        "MarshalHodgeMillennium", "MarshalHodgeUniversal", "MarshalYM", "MarshalYMMillennium",
        "MarshalYMUniversal", "MarshalGoldbach"};

    for (const auto& m : bundle.merged_modules) {

        for (const auto& g : m.proof_graphs) {

            if (!kLadderGraphs.count(g.name)) continue;

            for (const auto& ob : g.obligations) {

                if ((ob.proof_class == ProofClass::Analytic ||
                     ob.proof_class == ProofClass::ClassicalImport ||
                     ob.proof_class == ProofClass::Reduction ||
                     ob.proof_class == ProofClass::AnalyticOpen ||

                     ob.proof_class == ProofClass::Universal ||

                     ob.proof_class == ProofClass::Inductive ||

                     ob.proof_class == ProofClass::Convergent ||

                     ob.proof_class == ProofClass::Composition) &&

                    ob.prove_kind == MrsProofBodyKind::Infer) {

                    rep.infer_on_analytic_detected = true;

                    rep.infer_on_analytic.push_back(g.name + ":" + ob.id);

                }

            }

        }

    }



    for (const auto& name : obligation_refs) {

        const auto it = bodies.find(name);

        if (it == bodies.end()) continue;

        const std::string& body = it->second;

        if (!is_single_identifier_body(body)) continue;

        const std::string target = trim(body);

        if (!prove_names.count(target)) continue;

        rep.trivial_alias_detected = true;

        rep.trivial_aliases.push_back(name + " -> " + target);

    }



    std::unordered_map<std::string, std::vector<std::string>> adj;

    for (const auto& kv : bodies) {

        adj[kv.first] = prove_body_callees(kv.second);

    }



    std::unordered_set<std::string> visiting;

    std::unordered_set<std::string> visited;

    std::function<bool(const std::string&)> dfs = [&](const std::string& u) -> bool {

        if (visiting.count(u)) {

            rep.acyclic = false;

            rep.prove_cycle_path.push_back(u);

            return true;

        }

        if (visited.count(u)) return false;

        visiting.insert(u);

        for (const auto& v : adj[u]) {

            if (!prove_names.count(v)) continue;

            if (dfs(v)) {

                if (rep.prove_cycle_path.size() == 1 || rep.prove_cycle_path.front() != u)

                    rep.prove_cycle_path.insert(rep.prove_cycle_path.begin(), u);

                return true;

            }

        }

        visiting.erase(u);

        visited.insert(u);

        return false;

    };



    for (const auto& name : prove_names) {

        if (!visited.count(name) && dfs(name)) break;

    }



    if (rep.trivial_alias_detected || !rep.acyclic || rep.infer_on_analytic_detected) rep.ok = false;

    return rep;

}

MrsProveSpineReport validate_hadamard_prove_spine(const MrsCompilationBundle& bundle) {
    return validate_mrs_proof_discipline(bundle);
}

namespace {

bool obligation_exempt_from_witness(const std::string& id) {
    return id == "bsd_rh_prerequisite" || id == "hodge_rh_prerequisite" ||
           id == "goldbach_rh_prerequisite" || id == "ym_hodge_prerequisite" ||
           id == "goldbach_bsd_shared_gln2";
}

bool obligation_needs_prove_ref(const MrsProofObligationDecl& ob) {
    if (ob.prove_kind == MrsProofBodyKind::Infer) return false;
    if (ob.proof_class == ProofClass::Structural) return false;
    if (ob.proof_class == ProofClass::Composition) return true;
    if (ob.proof_class == ProofClass::Analytic || ob.proof_class == ProofClass::ClassicalImport ||
        ob.proof_class == ProofClass::Reduction || ob.proof_class == ProofClass::AnalyticOpen ||
        ob.proof_class == ProofClass::Numeric || ob.proof_class == ProofClass::NumericInterval ||
        ob.proof_class == ProofClass::Universal || ob.proof_class == ProofClass::Inductive ||
        ob.proof_class == ProofClass::Convergent || ob.proof_class == ProofClass::Rewrite ||
        ob.proof_class == ProofClass::DecisionProcedure)
        return true;
    return false;
}

std::string lower_ascii(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

bool contains_ident_token(const std::string& hay, const std::string& ident) {
    if (ident.empty()) return false;
    size_t pos = 0;
    while ((pos = hay.find(ident, pos)) != std::string::npos) {
        const bool left = pos == 0 || !is_ident_char(hay[pos - 1]);
        const bool right =
            pos + ident.size() >= hay.size() || !is_ident_char(hay[pos + ident.size()]);
        if (left && right) return true;
        pos += ident.size();
    }
    return false;
}

bool witness_is_trivial_gate(const std::string& witness_expr) {
    const std::string t = lower_ascii(trim(witness_expr));
    if (t.empty()) return false;
    if (t == "true" || t == "1") return true;
    static const char* kCapstoneOnly[] = {
        "bsd_rank_proved",      "hodge_conjecture_proved", "goldbach_proved",
        "ym_mass_gap_proved",   "classical_goldbach",      "classical_rh_ok",
        "proof_chain_closed",   "mrs_proof_audit_ok",
    };
    for (const char* flag : kCapstoneOnly) {
        if (t == flag) return true;
    }
    return false;
}

bool witness_embeds_capstone_for_obligation(const std::string& obligation_id,
                                            const std::string& witness_expr) {
    static const std::unordered_map<std::string, std::vector<std::string>> kForbidden = {
        {"bsd_rank_proved", {"bsd_rank_proved"}},
        {"classical_bsd_rank_general", {"bsd_rank_proved", "classical_bsd_rank_general"}},
        {"hodge_conjecture_proved", {"hodge_conjecture_proved"}},
        {"classical_hodge11_general", {"hodge_conjecture_proved", "classical_hodge11_general"}},
        {"goldbach_proved", {"goldbach_proved"}},
        {"classical_goldbach", {"goldbach_proved", "classical_goldbach"}},
        {"ym_mass_gap_proved", {"ym_mass_gap_proved"}},
        {"classical_ym_mass_gap_general", {"ym_mass_gap_proved", "classical_ym_mass_gap_general"}},
    };
    const auto it = kForbidden.find(obligation_id);
    if (it == kForbidden.end()) return false;
    for (const auto& tok : it->second) {
        if (contains_ident_token(witness_expr, tok)) return true;
    }
    return false;
}

bool prove_body_is_opaque_composition(const std::string& body,
                                      const std::unordered_set<std::string>& prove_names) {
    if (mrs_prove_body_is_formula_script(body)) return false;
    if (body.find("forall_extension(") != std::string::npos) return false;
    if (body.find("induction(") != std::string::npos) return false;
    if (body.find("convergence(") != std::string::npos) return false;
    const auto callees = prove_body_callees(body);
    int prove_callees = 0;
    for (const auto& c : callees) {
        if (prove_names.count(c)) ++prove_callees;
    }
    return prove_callees <= 1;
}

std::string squash_ws_lower(std::string s) {
    std::string out;
    bool space = false;
    for (char c : s) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            space = !out.empty();
            continue;
        }
        if (space) {
            out += ' ';
            space = false;
        }
        out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return trim(out);
}

bool witness_has_circular_identification(const std::string& expr) {
    static const char* kBanned[] = {
        "gamma_locked",     "gamma_tuned",       "uses_gamma",
        "height_locked",    "zero_height_ansatz", "round_gamma",
        "zeta_zero_input",  "gamma_ansatz",
    };
    const std::string hay = lower_ascii(expr);
    for (const char* tok : kBanned) {
        if (contains_ident_token(hay, tok)) return true;
    }
    return false;
}

bool witness_embeds_goal_equality(const std::string& witness_expr) {
    const std::string hay = lower_ascii(witness_expr);
    static const char* kPatterns[] = {
        "kernel_multiplicity == algebraic_rank",
        "kernel_multiplicity=algebraic_rank",
        "hodge_kernel_multiplicity == hodge_h11",
        "hodge_kernel_multiplicity=hodge_h11",
        "rank_match_ok and rank_match_ok",
    };
    for (const char* pat : kPatterns) {
        if (hay.find(pat) != std::string::npos) return true;
    }
    if (hay == "cycle_map_ok" || hay == "rank_match_ok" || hay == "hodge_match_ok") return true;
    return false;
}

bool rh_assumption_smuggled(const std::string& graph_name, const std::string& obligation_id,
                            const std::string& witness_expr) {
    if (graph_name == "MarshalHadamard") return false;
    if (obligation_exempt_from_witness(obligation_id)) return false;
    static const char* kRhFlags[] = {
        "classical_rh_ok", "rh_capstone_ok", "riemann_hypothesis_proved",
        "classical_riemann_hypothesis", "rh_proved", "classical_rh_cert",
    };
    for (const char* tok : kRhFlags) {
        if (contains_ident_token(witness_expr, tok)) return true;
    }
    return false;
}

bool prove_script_is_tautological(const MrsProveScript& script, const std::string& obligation_id,
                                  const std::string& prove_key) {
    if (!script.has_conclude || trim(script.conclusion).empty()) return true;
    if (prove_key == obligation_id && script.witness.empty() && script.classical.empty() &&
        script.deps.empty())
        return true;
    const std::string conc = squash_ws_lower(script.conclusion);
    const std::string oid = squash_ws_lower(obligation_id);
    if (conc == oid || conc.find(oid + " holds") != std::string::npos) return true;
    for (const auto& cl : script.classical) {
        if (squash_ws_lower(cl) == conc) return true;
    }
    return false;
}

bool prove_script_is_weak_reduction(const MrsProveScript& script) {
    if (!script.steps.empty()) return false;
    return script.witness.empty() && script.classical.empty() && !script.deps.empty();
}

bool prove_script_has_proof_sketch(const MrsProveScript& script) {
    const std::string c = squash_ws_lower(script.conclusion);
    return c.find("(proof:") != std::string::npos || c.find("(analytic:") != std::string::npos ||
           c.find("(proved") != std::string::npos;
}

bool prove_script_missing_explicit_steps(const MrsProveScript& script, ProofClass pc) {
    if (pc != ProofClass::Composition) return false;
    if (!script.steps.empty()) return false;
    if (!script.classical.empty()) return false;
    return !prove_script_has_proof_sketch(script);
}

bool prove_script_assume_mentions_target(const MrsProveScript& script,
                                         const std::string& obligation_id,
                                         const std::string& prove_key) {
    const std::string target = lower_ascii(obligation_id);
    const std::string alias = lower_ascii(prove_key);
    auto has_target = [&](const std::string& s) {
        const std::string n = lower_ascii(s);
        return (!target.empty() && contains_ident_token(n, target)) ||
               (!alias.empty() && contains_ident_token(n, alias));
    };
    for (const auto& cl : script.classical) {
        if (has_target(cl)) return true;
    }
    for (const auto& w : script.witness) {
        if (has_target(w)) return true;
    }
    return false;
}

bool prove_script_exactness_from_tolerance_only(const MrsProveScript& script,
                                                const std::string& obligation_id) {
    const std::string oid = squash_ws_lower(obligation_id);
    const bool exact_sensitive = oid.find("exact_grid") != std::string::npos ||
                                 oid.find("identity_theorem") != std::string::npos;
    if (!exact_sensitive) return false;
    const std::string c = squash_ws_lower(script.conclusion);
    const bool asks_exact =
        c.find(" exactly") != std::string::npos || c.find(" = ") != std::string::npos ||
        c.find("⊢") != std::string::npos;
    if (!asks_exact) return false;
    if (!script.classical.empty() || !script.deps.empty()) return false;

    bool saw_tol = false;
    for (const auto& w : script.witness) {
        const std::string nw = squash_ws_lower(w);
        if (nw.find("_ub") != std::string::npos || nw.find("tolerance") != std::string::npos ||
            nw.find("gap") != std::string::npos || nw.find("pin") != std::string::npos) {
            saw_tol = true;
        } else {
            return false;
        }
    }
    return saw_tol;
}

void validate_obligation_dep_graph(const MrsCompilationBundle& bundle, MrsProveSpineReport* rep) {
    if (!rep) return;
    std::unordered_map<std::string, std::vector<std::string>> adj;
    std::unordered_set<std::string> nodes;
    for (const auto& m : bundle.merged_modules) {
        for (const auto& g : m.proof_graphs) {
            for (const auto& ob : g.obligations) {
                nodes.insert(ob.id);
                adj[ob.id] = ob.dependencies;
            }
        }
    }
    std::unordered_set<std::string> visiting;
    std::unordered_set<std::string> visited;
    std::function<bool(const std::string&)> dfs = [&](const std::string& u) -> bool {
        if (visiting.count(u)) {
            rep->obligation_graph_acyclic = false;
            rep->obligation_cycle_path.push_back(u);
            return true;
        }
        if (visited.count(u)) return false;
        visiting.insert(u);
        for (const auto& v : adj[u]) {
            if (!nodes.count(v)) continue;
            if (dfs(v)) {
                if (rep->obligation_cycle_path.size() == 1 || rep->obligation_cycle_path.front() != u)
                    rep->obligation_cycle_path.insert(rep->obligation_cycle_path.begin(), u);
                return true;
            }
        }
        visiting.erase(u);
        visited.insert(u);
        return false;
    };
    for (const auto& id : nodes) {
        if (!visited.count(id) && dfs(id)) break;
    }
}

}  // namespace

bool witness_expr_passes_hardening(const std::string& graph_name,
                                   const std::string& obligation_id,
                                   const std::string& witness_expr, std::string* reason) {
    if (witness_expr.empty()) return true;
    if (obligation_exempt_from_witness(obligation_id)) return true;
    if (contains_ident_token(witness_expr, obligation_id)) {
        if (reason) *reason = "witness_expr self-references obligation_id " + obligation_id;
        return false;
    }
    if (witness_is_trivial_gate(witness_expr)) {
        if (reason) *reason = "witness_expr is trivial capstone gate";
        return false;
    }
    if (witness_embeds_capstone_for_obligation(obligation_id, witness_expr)) {
        if (reason) *reason = "witness_expr embeds capstone conclusion for " + obligation_id;
        return false;
    }
    if (witness_has_circular_identification(witness_expr)) {
        if (reason) *reason = "witness_expr uses circular identification token";
        return false;
    }
    if (witness_expr.find("exact_grid_pin_deprecated") != std::string::npos ||
        witness_expr.find("structural_grid_pin_ok") != std::string::npos) {
        if (reason) *reason = "witness_expr uses deprecated structural pin token";
        return false;
    }
    if (witness_embeds_goal_equality(witness_expr)) {
        if (reason) *reason = "witness_expr embeds capstone equality (use inequality gates)";
        return false;
    }
    if (rh_assumption_smuggled(graph_name, obligation_id, witness_expr)) {
        if (reason) *reason = "witness_expr smuggles RH capstone flag outside structural prereq";
        return false;
    }
    return true;
}

MrsProveSpineReport validate_mrs_proof_discipline(const MrsCompilationBundle& bundle) {
    MrsProveSpineReport rep = validate_ladder_prove_spine(bundle);
    const auto bodies = collect_prove_bodies(bundle);

    std::unordered_set<std::string> prove_names;
    for (const auto& m : bundle.merged_modules) {
        for (const auto& p : m.proves) {
            if (!p.name.empty()) prove_names.insert(p.name);
        }
    }

    for (const auto& m : bundle.merged_modules) {
        for (const auto& g : m.proof_graphs) {
            for (const auto& ob : g.obligations) {
                const std::string tag = g.name + ":" + ob.id;

                if (!obligation_exempt_from_witness(ob.id) && ob.witness_expr.empty()) {
                    rep.missing_witness_expr.push_back(tag);
                    rep.ok = false;
                }

                for (const auto& dep : ob.dependencies) {
                    if (dep == ob.id) {
                        rep.circular_witness_detected = true;
                        rep.circular_witness.push_back(tag + ":obligation_dep_self");
                        rep.ok = false;
                    }
                }

                if (!obligation_exempt_from_witness(ob.id) && !ob.witness_expr.empty()) {
                    std::string hard_err;
                    if (!witness_expr_passes_hardening(g.name, ob.id, ob.witness_expr, &hard_err)) {
                        if (hard_err.find("self-reference") != std::string::npos ||
                            hard_err.find("self-references") != std::string::npos) {
                            rep.circular_witness_detected = true;
                            rep.circular_witness.push_back(tag + ":" + hard_err);
                        } else if (hard_err.find("trivial") != std::string::npos) {
                            rep.weak_witness_detected = true;
                            rep.weak_witness.push_back(tag + ":" + hard_err);
                        } else if (hard_err.find("circular identification") != std::string::npos) {
                            rep.circular_identification_detected = true;
                            rep.circular_identification.push_back(tag + ":" + hard_err);
                        } else if (hard_err.find("equality") != std::string::npos) {
                            rep.goal_equality_in_witness_detected = true;
                            rep.goal_equality_in_witness.push_back(tag + ":" + hard_err);
                        } else if (hard_err.find("RH capstone") != std::string::npos) {
                            rep.rh_assumption_smuggle_detected = true;
                            rep.rh_assumption_smuggle.push_back(tag + ":" + hard_err);
                        } else {
                            rep.capstone_in_witness_detected = true;
                            rep.capstone_in_witness.push_back(tag + ":" + hard_err);
                        }
                        rep.ok = false;
                    }
                }

                if (!ob.statement.empty() && !ob.witness_expr.empty() &&
                    squash_ws_lower(ob.statement) == squash_ws_lower(ob.witness_expr)) {
                    rep.tautological_prove_detected = true;
                    rep.tautological_prove.push_back(tag + ":statement_equals_witness_expr");
                    rep.ok = false;
                }

                const std::string prove_key =
                    !ob.prove_ref.empty() ? ob.prove_ref : ob.extend_via;
                if (prove_key == ob.id && ob.proof_class != ProofClass::Universal) {
                    const auto it_rename = bodies.find(prove_key);
                    if (it_rename != bodies.end()) {
                        const std::string& body_rename = it_rename->second;
                        if (is_single_identifier_body(body_rename) ||
                            (mrs_prove_body_is_formula_script(body_rename) &&
                             prove_script_is_tautological(parse_mrs_prove_script(body_rename),
                                                          ob.id, prove_key))) {
                            rep.tautological_prove_detected = true;
                            rep.tautological_prove.push_back(tag + ":prove_ref_equals_obligation_id");
                            rep.ok = false;
                        }
                    }
                }

                if (obligation_needs_prove_ref(ob) && ob.prove_ref.empty() &&
                    ob.extend_via.empty()) {
                    rep.missing_prove_ref.push_back(tag);
                    rep.ok = false;
                }

                if (prove_key.empty()) continue;

                const auto it = bodies.find(prove_key);
                if (it == bodies.end()) {
                    rep.undisciplined_prove.push_back(tag + ":missing_prove_body:" + prove_key);
                    rep.ok = false;
                    continue;
                }
                const std::string& body = it->second;

                if (ob.proof_class == ProofClass::Analytic ||
                    ob.proof_class == ProofClass::ClassicalImport ||
                    ob.proof_class == ProofClass::Reduction ||
                    ob.proof_class == ProofClass::AnalyticOpen) {
                    if (!mrs_prove_body_is_formula_script(body)) {
                        rep.infer_on_analytic_detected = true;
                        rep.infer_on_analytic.push_back(tag + ":analytic_requires_script");
                        rep.ok = false;
                    }
                } else if (ob.proof_class == ProofClass::Universal) {
                    if (body.find("forall_extension(") == std::string::npos) {
                        rep.undisciplined_prove.push_back(tag + ":universal_requires_forall_extension");
                        rep.ok = false;
                    }
                } else if (ob.proof_class == ProofClass::Inductive) {
                    if (body.find("induction(") == std::string::npos) {
                        rep.undisciplined_prove.push_back(tag + ":inductive_requires_induction");
                        rep.ok = false;
                    }
                } else if (ob.proof_class == ProofClass::Convergent) {
                    if (body.find("convergence(") == std::string::npos) {
                        rep.undisciplined_prove.push_back(tag + ":convergent_requires_convergence");
                        rep.ok = false;
                    }
                } else if (ob.proof_class == ProofClass::Composition) {
                    if (!mrs_prove_body_is_disciplined(body, prove_names)) {
                        rep.undisciplined_prove.push_back(tag + ":undisciplined_prove_body");
                        rep.ok = false;
                    }
                    if (prove_body_is_opaque_composition(body, prove_names)) {
                        rep.opaque_composition_detected = true;
                        rep.opaque_composition.push_back(tag + ":" + prove_key);
                        rep.ok = false;
                    }
                }

                if (mrs_prove_body_is_formula_script(body)) {
                    const MrsProveScript script = parse_mrs_prove_script(body);
                    if (!script.has_conclude || script.conclusion.empty()) {
                        rep.undisciplined_prove.push_back(tag + ":empty_conclude");
                        rep.ok = false;
                    }
                    if (prove_script_is_tautological(script, ob.id, prove_key)) {
                        rep.tautological_prove_detected = true;
                        rep.tautological_prove.push_back(tag + ":" + prove_key);
                        rep.ok = false;
                    }
                    if ((ob.proof_class == ProofClass::Analytic ||
                         ob.proof_class == ProofClass::Reduction ||
                         ob.proof_class == ProofClass::ClassicalImport) &&
                        prove_script_is_weak_reduction(script)) {
                        rep.weak_analytic_reduction_detected = true;
                        rep.weak_analytic_reduction.push_back(tag + ":" + prove_key);
                        rep.ok = false;
                    }
                    if ((ob.proof_class == ProofClass::Analytic ||
                         ob.proof_class == ProofClass::Reduction ||
                         ob.proof_class == ProofClass::ClassicalImport) &&
                        script.classical.empty() &&
                        script.witness.empty() && script.deps.empty()) {
                        rep.undisciplined_prove.push_back(tag + ":analytic_script_missing_assume");
                        rep.ok = false;
                    }
                    if (prove_script_assume_mentions_target(script, ob.id, prove_key)) {
                        rep.assume_target_leak_detected = true;
                        rep.assume_target_leak.push_back(tag + ":" + prove_key);
                        rep.ok = false;
                    }
                    if (prove_script_exactness_from_tolerance_only(script, ob.id)) {
                        rep.weak_analytic_reduction_detected = true;
                        rep.weak_analytic_reduction.push_back(
                            tag + ":" + prove_key + ":exactness_from_tolerance_only");
                        rep.ok = false;
                    }
                    if (prove_script_missing_explicit_steps(script, ob.proof_class)) {
                        rep.missing_explicit_steps_detected = true;
                        rep.missing_explicit_steps.push_back(tag + ":" + prove_key);
                        rep.ok = false;
                    }
                    std::string dep_err;
                    if (!validate_prove_script_deps(script, ob.dependencies, &dep_err)) {
                        rep.script_dep_mismatch.push_back(tag + ":" + dep_err);
                        rep.ok = false;
                    }
                }
            }
        }
    }

    validate_obligation_dep_graph(bundle, &rep);
    if (!rep.obligation_graph_acyclic) rep.ok = false;

    return rep;
}

}  // namespace Marshal::AnaVM

