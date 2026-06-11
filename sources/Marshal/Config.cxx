#include "Config.hxx"
#include "Compat.hxx"

namespace Marshal {

SimdLevel DetectSimd() {
#if defined(WEIL_HAVE_AVX2)
    return SimdLevel::AVX2;
#else
    return SimdLevel::Scalar;
#endif
}

const char* SimdName(SimdLevel l) {
    return l == SimdLevel::AVX2 ? "AVX2" : "scalar";
}

const char* ZeroKernelName(ZeroKernel k) {
    return k == ZeroKernel::LongDouble ? "long-double" : "float-simd";
}

}  // namespace Marshal
