// Unit tests — MrsKernel + MrsInfer on toy rational bounds.

#include "AnaVM/AnaVm.hxx"
#include "AnaVM/MrsInfer.hxx"

#include <filesystem>
#include <iostream>

namespace {

int g_fails = 0;

void fail(const char* msg) {
    std::cerr << "FAIL: " << msg << "\n";
    ++g_fails;
}

void require(bool cond, const char* msg) {
    if (!cond) fail(msg);
}

void test_infer_prove_direct() {
    using namespace Marshal::AnaVM;
    MrsProveDecl p;
    p.name = "moment_tolerance_ok";
    p.type_expr = "MOMENT_TOLERANCE < Rational(1, 100)";
    p.body_kind = MrsProofBodyKind::Infer;
    MrsInferAuditEntry entry;
    std::string err;
    require(try_infer_prove(p, {}, &entry, &err), err.c_str());
    require(entry.ok, "audit entry ok");
}

void test_infer_audit_export() {
    using namespace Marshal::AnaVM;
    MrsInferReport rep;
    rep.ok = true;
    MrsInferAuditEntry e;
    e.obligation_id = "test";
    e.source = "unit";
    e.rational_num = "1";
    e.rational_den = "1000";
    e.ok = true;
    rep.audit.push_back(e);
    std::filesystem::create_directories("build/test_out");
    require(export_infer_audit_json("build/test_out/mrs_infer_audit.json", rep),
            "export infer audit json");
}

}  // namespace

int main() {
    test_infer_prove_direct();
    test_infer_audit_export();
    if (g_fails) {
        std::cerr << g_fails << " test(s) failed\n";
        return 1;
    }
    std::cout << "MrsKernelTest: all passed\n";
    return 0;
}
