#pragma once
#include <string>
#include <vector>
#include "Numerics/Real.hxx"
bool LoadZerosFast(const std::string& path, std::vector<double>& out, size_t max_count, bool use_cache, std::vector<Real>* out_ld = nullptr);
void PromoteZerosLd(const std::vector<double>& in, std::vector<Real>& out);
