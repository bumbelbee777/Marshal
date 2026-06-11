#include "TraceApi.hxx"

namespace Marshal {

Real TraceResult::residual() const {
    return residual_kahan != 0.0L ? residual_kahan : (lhs - rhs);
}

}  // namespace Marshal
