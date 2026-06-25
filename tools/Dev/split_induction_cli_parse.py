#!/usr/bin/env python3
"""Split ParseConfig else-if chain into two flat-if helpers for MSVC C1061."""
from pathlib import Path

path = Path("sources/Marshal/Induction/InductionCli.cxx")
text = path.read_text(encoding="utf-8")
start = text.index("bool ParseConfig(int argc")
end = text.index("\nvoid PrintResult(Real sigma")
head, tail = text[:start], text[end:]
block = text[start:end]

lambda_end = block.index("        };\n") + len("        };\n")
body_end = block.rindex("        if (!err.empty()) return false;")
body = block[lambda_end:body_end]

branches: list[list[str]] = []
current: list[str] | None = []
for line in body.splitlines():
    stripped = line.strip()
    if stripped.startswith("else if ") or (stripped.startswith("if (") and not current):
        if current:
            branches.append(current)
        if stripped.startswith("else if "):
            line = line.replace("else if ", "if ", 1)
        current = [line]
    elif stripped.startswith("else {"):
        if current:
            branches.append(current)
        current = []
    elif stripped.startswith("} else if "):
        if current:
            current.append("        }")
            branches.append(current)
        line = line.replace("} else if ", "if ", 1)
        current = [line]
    elif current is not None:
        current.append(line)

if current:
    branches.append(current)

split_at = next(
    i
    for i, br in enumerate(branches)
    if any('"--anavm"' in ln for ln in br)
)
a_branches = branches[:split_at]
b_branches = [
    br for br in branches[split_at:]
    if not any('"-h"' in ln or '"--help"' in ln or "Unknown:" in ln for ln in br)
]


def wrap_branch(lines: list[str]) -> str:
    out = list(lines)
    for idx in range(len(out) - 1, -1, -1):
        if out[idx].strip() == "}":
            out.insert(idx, "            return true;")
            return "\n".join(out)
    out[-1] = out[-1].rstrip()
    if not out[-1].endswith(";"):
        out[-1] += ";"
    out[-1] += " return true;"
    return "\n".join(out)


def emit_fn(name: str, branch_list: list[list[str]]) -> str:
    hdr = [
        f"static bool {name}(const std::string& arg, Config& cfg,",
        "                                 int& i, int argc, char** argv, std::string& err) {",
        "    auto need = [&](const char* name) -> std::string {",
        '        if (i + 1 >= argc) { err = std::string("Missing value for ") + name; return {}; }',
        "        return argv[++i];",
        "    };",
    ]
    parts = hdr + [wrap_branch(br) for br in branch_list]
    parts += ["    return false;", "}"]
    return "\n".join(parts)


helpers = emit_fn("ParseConfigFlagsA", a_branches) + "\n\n" + emit_fn("ParseConfigFlagsB", b_branches)
new_parse = """bool ParseConfig(int argc, char** argv, Config& cfg, std::string& err) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (ParseConfigFlagsA(arg, cfg, i, argc, argv, err) ||
            ParseConfigFlagsB(arg, cfg, i, argc, argv, err)) {
            if (!err.empty()) return false;
            continue;
        }
        if (arg == "-h" || arg == "--help") { PrintUsage(argv[0]); std::exit(0); }
        err = std::string("Unknown: ") + arg;
        return false;
    }
    return validate_config(cfg.sigma, cfg.prime_limit, cfg.do_sweep, cfg.sweep_min,
                           cfg.sweep_max, cfg.sweep_steps, cfg.max_zeros,
                           cfg.kmax, cfg.nmax, cfg.ktheta, err);
}
"""

path.write_text(head + helpers + "\n\n" + new_parse + tail, encoding="utf-8", newline="\n")
print(f"Split {len(a_branches)} + {len(b_branches)} branches")
