# Hodge conjecture outlook — K3 / Hitchin moduli

**Status:** EVIDENCE-tier demonstration — not a proof of the Hodge conjecture.

Cross-links: [GLnPlugAndPlayArchitecture.md](GLnPlugAndPlayArchitecture.md), [GrandUnificationManifesto.md](GrandUnificationManifesto.md).

---

## Reduction statement (outlook)

On a K3 surface \(X\), Hodge classes in \(H^{1,1}(X,\mathbb{Q})\) are conjecturally algebraic cycles. The **Hitchin moduli space** \(\mathcal{M}_{\mathrm{Hitchin}}\) associated to \(X\) carries a spectral triple \((\mathcal{A},\mathcal{H},D)\). The Marshal demonstration posits:

\[
\text{(1,1) Hodge classes} \longleftrightarrow \ker D \cap \mathcal{H}^{(1,1)}.
\]

Full Hitchin integrable-system geometry is deferred; rank-3 `MarshalGLnDirac` with preset `hitchin_k3_stub` exports `marshal_hodge_k3_demo.json` with `kernel_multiplicity = h^{1,1} = 20`.

---

## Cert fields

| Field | Role |
|-------|------|
| `hodge_index` | \(\{h^{2,0}, h^{1,1}, h^{0,2}\}\) on K3 stub |
| `predicted_hodge_multiplicity` | Expected \(\dim \ker D\) in (1,1) sector |
| `kernel_multiplicity` | Marshal count at tolerance \(\varepsilon\) |
| `hodge_match` | Boolean gate for evidence tier |

Emit: `python tools/Analysis/HodgeK3Demo.py --check`
