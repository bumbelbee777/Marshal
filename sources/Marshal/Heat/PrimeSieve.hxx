#pragma once
#include <vector>

namespace Marshal::Heat {

// Segmented odd-prime bitmap sieve; OpenMP-parallel segments for large limits.
std::vector<int> SievePrimes(int max_n);

}  // namespace Marshal::Heat
