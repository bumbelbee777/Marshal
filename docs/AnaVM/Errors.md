# AnaVM error codes



| Code | Meaning |

|------|---------|

| E0100 | Parse / file error |

| E0400 | `gamma` without `uses_gamma: true` |

| E0401 | `gamma` shadowing banned |

| E0600 | Weil `poisson` incompatible with derived ω |

| E0602 | Placeholder ansatz + non-none coupling |

| E0603 | Placeholder space without `sym_tier: scaffold` |

| E0700 | `trace_formula` present but `status != proved` on production ansatz |

| E0701 | `self_adjoint_extension` on production ansatz without analytic backend |

| E0702 | `berry_keating` coupling on non-phase-space ansatz |

| E0800 | AnaVM proof graph contains a cycle (`circular_logic_detected`) |

| E0801 | Proof obligation failed numeric / analytic gate |

| E0802 | Forbidden `hadamardWeierstrassIdentificationClosed` JSON boolean in cert export |

| E0900 | MRS `mod`/`use` import cycle detected |

| E0901 | MrsInfer could not discharge `{ infer }` obligation (no silent stub) |

| E0902–E0918 | MRS prove-spine discipline (see [MrsLanguage.md](MrsLanguage.md)) |



`E0600` compares derived spectrum from `MrsSym` to `2*pi*n/log(p)` for production cylinder programs.

Scaffold programs (Berry–Keating, Connes) skip `E0600` and export `formal_calibration.json`.

Analytic programs (`connes_analytic_construction`, `berry_keating_xp` with semiclassical) skip `E0600`.



**XiHadamard proof engine:** `programs/marshal_xi_hadamard.mrs` + `--xi-hadamard-proof` runs the acyclic

Marshal Hadamard closure spine in C++ (`AnaProofEngine` + `XiHadamardEngine`). Audit sync only —

`verify-xi-hadamard` does **not** gate Lean `xi_publication_classical_rh_unconditional`. See [MarshalXiHadamardPublication.md](../Analysis/MarshalXiHadamardPublication.md).

