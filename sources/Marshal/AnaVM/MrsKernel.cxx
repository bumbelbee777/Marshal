#include "MrsKernel.hxx"

#include "AnaProofEngine.hxx"

namespace Marshal::AnaVM {

MrsKernelReport check_compilation_bundle(MrsCompilationBundle& bundle) {
    MrsKernelReport rep;
    for (const auto& m : bundle.merged_modules) {
        for (const auto& p : m.proves) {
            if (p.name.empty()) continue;
            if (p.body_kind == MrsProofBodyKind::Infer) {
                ++rep.inferred_count;
            } else if (p.body_kind == MrsProofBodyKind::Explicit) {
                if (p.body.empty()) {
                    MrsError e;
                    e.code = "E0901";
                    e.span.line = p.span.line;
                    e.message = "empty explicit proof body for " + p.name;
                    rep.errors.push_back(e);
                    continue;
                }
                ++rep.proved_count;
            } else {
                ++rep.proved_count;
            }
        }
        for (const auto& g : m.proof_graphs) {
            ProofGraphReport pg;
            pg.target_theorem = g.target;
            pg.architecture = g.architecture;
            for (const auto& ob : g.obligations) {
                ProofObligation o;
                o.id = ob.id;
                o.statement = ob.statement;
                o.lean_theorem = ob.lean_theorem;
                o.proof_class = ob.proof_class;
                o.dependencies = ob.dependencies;
                if (ob.prove_kind == MrsProofBodyKind::Infer)
                    o.evidence = "mrs_infer_pending";
                else if (!ob.prove_ref.empty())
                    o.evidence = "mrs_prove_ref:" + ob.prove_ref;
                pg.obligations.push_back(std::move(o));
            }
            pg.acyclic = !proof_graph_has_cycle(pg, &pg.cycle_path);
            pg.circular_logic_detected = !pg.acyclic;
            if (!pg.acyclic) {
                MrsError e;
                e.code = "E0900";
                e.message = "proof_graph " + g.name + " has cycle";
                rep.errors.push_back(e);
            }
        }
    }
    rep.ok = rep.errors.empty();
    return rep;
}

}  // namespace Marshal::AnaVM
