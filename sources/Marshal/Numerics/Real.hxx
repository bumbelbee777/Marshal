#pragma once
// Extended-precision Real type for Marshal Weil balance.

#include <cmath>
#include <cstdint>

#if defined(MARSHAL_USE_FLOAT128) && defined(__SIZEOF_FLOAT128__)
using Real = __float128;
#define MARSHAL_LIT(x) (static_cast<Real>(x))
inline Real MarshalExp(Real x) {
#if defined(__GNUC__) && defined(__builtin_expq)
    return __builtin_expq(x);
#else
    return static_cast<Real>(expl(static_cast<long double>(x)));
#endif
}
inline Real MarshalSqrt(Real x) { return static_cast<Real>(sqrtl(static_cast<long double>(x))); }
inline Real MarshalLog(Real x) { return static_cast<Real>(logl(static_cast<long double>(x))); }
inline Real MarshalFabs(Real x) { return static_cast<Real>(fabsl(static_cast<long double>(x))); }
inline const char* MarshalRealName() { return "float128"; }
#else
using Real = long double;
#define MARSHAL_LIT(x) (static_cast<Real>(x##L))
inline Real MarshalExp(Real x) { return expl(x); }
inline Real MarshalSqrt(Real x) { return sqrtl(x); }
inline Real MarshalLog(Real x) { return logl(x); }
inline Real MarshalFabs(Real x) { return fabsl(x); }
inline const char* MarshalRealName() { return "long-double"; }
#endif

inline int MarshalRealBits() {
#if defined(MARSHAL_USE_FLOAT128) && defined(__SIZEOF_FLOAT128__)
    return static_cast<int>(__SIZEOF_FLOAT128__ * 8);
#else
    return static_cast<int>(sizeof(Real) * 8);
#endif
}
