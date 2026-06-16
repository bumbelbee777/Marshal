#pragma once
#include <cstddef>
#include <string>
#include <vector>
#include "Numerics/Real.hxx"

bool LoadZerosFast(const std::string& path, std::vector<double>& out, size_t max_count,
                   bool use_cache, std::vector<Real>* out_ld = nullptr);
bool LoadZerosBinary(const std::string& path, std::vector<double>& out, size_t max_count);
bool SaveZerosBinary(const std::string& path, const std::vector<double>& data);
void PromoteZerosLd(const std::vector<double>& in, std::vector<Real>& out);
