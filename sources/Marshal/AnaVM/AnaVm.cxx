#include "AnaVm.hxx"

namespace Marshal::AnaVM {

CompileResult compile_program(const std::string& path, bool /*check_only*/) {
    CompileResult r;
    ParseResult pr = parse_mrs_file(path);
    r.program = std::move(pr.program);
    r.errors = std::move(pr.errors);
    r.ok = pr.ok;
    return r;
}

}  // namespace Marshal::AnaVM
