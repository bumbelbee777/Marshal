import Lake
open Lake DSL

package HP where
  leanOptions := #[⟨`autoImplicit, false⟩]

@[default_target]
lean_lib HP where
  roots := #[`HPWeil, `AdeleQuotient, `CylinderNoGo, `PoissonDuality]
