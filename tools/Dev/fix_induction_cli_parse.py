#!/usr/bin/env python3
"""Rewrite ParseConfig else-if chain to flat if+goto for MSVC block nesting limit."""
from pathlib import Path

path = Path("sources/Marshal/Induction/InductionCli.cxx")
text = path.read_text(encoding="utf-8")
start = text.index("bool ParseConfig(int argc")
end = text.index("\nvoid PrintResult(Real sigma")
head, rest = text[:start], text[end:]
block = text[start:end]

block = block.replace("        else if (", "        if (")
block = block.replace("        } else if (", "        } if (")

# Append goto arg_done to each single-line if handler.
lines = block.splitlines(keepends=True)
out = []
i = 0
while i < len(lines):
    line = lines[i]
    stripped = line.strip()
    if stripped.startswith("if (arg ==") or stripped.startswith("if (arg !="):
        # Collect block: single line or braced block
        if stripped.endswith("{"):
            out.append(line)
            i += 1
            depth = 1
            while i < len(lines) and depth > 0:
                out.append(lines[i])
                depth += lines[i].count("{") - lines[i].count("}")
                i += 1
            if not out[-1].strip().startswith("arg_done"):
                out.append("        goto arg_done;\n")
            continue
        if ";" in stripped or stripped.endswith("true") or stripped.endswith("false"):
            if "goto arg_done" not in stripped:
                out.append(line.rstrip("\n") + " goto arg_done;\n")
            else:
                out.append(line)
            i += 1
            continue
        # multiline without brace on same line
        out.append(line)
        i += 1
        while i < len(lines):
            nxt = lines[i]
            out.append(nxt)
            if ";" in nxt.strip() or nxt.strip().endswith("}"):
                if not any("goto arg_done" in x for x in out[-3:]):
                    out.append("        goto arg_done;\n")
                i += 1
                break
            i += 1
        continue
    out.append(line)
    i += 1

block = "".join(out)
block = block.replace(
    '        else { err = std::string("Unknown: ") + arg; return false; }\n'
    '        if (!err.empty()) return false;',
    '        err = std::string("Unknown: ") + arg;\n'
    '        return false;\n'
    '        arg_done:\n'
    '        if (!err.empty()) return false;',
)

path.write_text(head + block + rest, encoding="utf-8", newline="\n")
print("Updated", path)
