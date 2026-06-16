#include "ZeroVerify.hxx"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void usage(const char* prog) {
    std::cerr << "Usage: " << prog
              << " --zeros <path> [--count N] [--offset O] [--tol T] [--threads T]"
                 " [--output json] [--max-count M]\n";
}

}  // namespace

int main(int argc, char** argv) {
    Marshal::Ntz::ZeroVerifyOptions opt;
    opt.zeros_path = "tests/Fixtures/Zeros/NtzMergedOneLine.txt";
    opt.output_json = "docs/generated/marshal_odilyzko_zero_cert.json";

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        auto need = [&](const char* flag) -> bool {
            if (a != flag) return false;
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << flag << "\n";
                std::exit(2);
            }
            return true;
        };
        if (need("--zeros")) {
            opt.zeros_path = argv[++i];
        } else if (need("--output")) {
            opt.output_json = argv[++i];
        } else if (need("--count")) {
            opt.count = static_cast<size_t>(std::strtoull(argv[++i], nullptr, 10));
        } else if (need("--offset")) {
            opt.offset = static_cast<size_t>(std::strtoull(argv[++i], nullptr, 10));
        } else if (need("--max-count")) {
            opt.max_count = static_cast<size_t>(std::strtoull(argv[++i], nullptr, 10));
        } else if (need("--tol")) {
            opt.z_tol = std::strtod(argv[++i], nullptr);
        } else if (need("--threads")) {
            opt.threads = std::atoi(argv[++i]);
        } else if (a == "--help" || a == "-h") {
            usage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown arg: " << a << "\n";
            usage(argv[0]);
            return 2;
        }
    }

    Marshal::Ntz::ZeroVerifyReport rep;
    std::string err;
    if (!Marshal::Ntz::RunZeroVerify(opt, rep, err)) {
        std::cerr << err << "\n";
        return 1;
    }
    if (!Marshal::Ntz::WriteZeroVerifyJson(opt.output_json, opt, rep, err)) {
        std::cerr << err << "\n";
        return 1;
    }

    std::cout << "ZeroVerify: certified=" << rep.certified_count << " verified=" << rep.verified_count
              << " max|Z|=" << rep.max_z_abs_refined << " ms=" << rep.elapsed_ms << "\n";
    std::cout << "Wrote " << opt.output_json << "\n";
    return rep.all_verified ? 0 : 1;
}
