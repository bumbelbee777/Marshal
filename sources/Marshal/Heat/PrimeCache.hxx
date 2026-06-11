#pragma once
#include <string>
#include <vector>

namespace Marshal::Heat {

std::vector<int> LoadOrSievePrimes(int max_n, const std::string& cache_dir = "build/cache");

}  // namespace Marshal::Heat
