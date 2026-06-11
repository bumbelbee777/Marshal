#include "PrimeCache.hxx"
#include "PrimeSieve.hxx"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace Marshal::Heat {

namespace {

constexpr uint32_t kCacheVersion = 1;
constexpr uint64_t kCacheMagic = 0x4D524C50524D455ULL;  // MRLPRME

std::string cache_path(int max_n, const std::string& cache_dir) {
    std::ostringstream oss;
    oss << cache_dir << "/primes_" << max_n << ".bin";
    return oss.str();
}

bool load_cache(const std::string& path, int max_n, std::vector<int>& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return false;
    }
    uint64_t magic = 0;
    uint32_t version = 0;
    int limit = 0;
    uint32_t count = 0;
    in.read(reinterpret_cast<char*>(&magic), 8);
    in.read(reinterpret_cast<char*>(&version), 4);
    in.read(reinterpret_cast<char*>(&limit), 4);
    in.read(reinterpret_cast<char*>(&count), 4);
    if (!in || magic != kCacheMagic || version != kCacheVersion || limit != max_n || count == 0) {
        return false;
    }
    out.resize(count);
    in.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(count * sizeof(int)));
    return static_cast<uint32_t>(in.gcount()) == count * sizeof(int);
}

void save_cache(const std::string& path, int max_n, const std::vector<int>& primes) {
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return;
    }
    const uint64_t magic = kCacheMagic;
    const uint32_t version = kCacheVersion;
    const int limit = max_n;
    const uint32_t count = static_cast<uint32_t>(primes.size());
    out.write(reinterpret_cast<const char*>(&magic), 8);
    out.write(reinterpret_cast<const char*>(&version), 4);
    out.write(reinterpret_cast<const char*>(&limit), 4);
    out.write(reinterpret_cast<const char*>(&count), 4);
    out.write(reinterpret_cast<const char*>(primes.data()),
              static_cast<std::streamsize>(count * sizeof(int)));
}

}  // namespace

std::vector<int> LoadOrSievePrimes(int max_n, const std::string& cache_dir) {
    const std::string path = cache_path(max_n, cache_dir);
    std::vector<int> primes;
    if (load_cache(path, max_n, primes)) {
        return primes;
    }
    primes = SievePrimes(max_n);
    save_cache(path, max_n, primes);
    return primes;
}

}  // namespace Marshal::Heat
