# Marshal MCP — epistemic rigor server

## Enable in Cursor

Project config: `.cursor/mcp.json` (already wired).

If the server does not appear, add to user `~/.cursor/mcp.json`:

```json
{
  "mcpServers": {
    "marshal-rigor": {
      "type": "stdio",
      "command": "python",
      "args": ["${workspaceFolder}/tools/mcp/marshal_rigor_mcp.py"]
    }
  }
}
```

Restart Cursor or reload MCP after changes.

## Tools

| Tool | When to use |
|------|-------------|
| `rigor_preflight_closure` | Before marking PROVED / closing capstone / cert emit (`mrs_obligation`, `mrs_capstone`, `cert_emit`, `fortress`) |
| `rigor_audit_file` | After editing `.mrs`, cert `.json`, or analysis docs |
| `rigor_audit_tree` | After proof-spine or ladder edits |

## CLI equivalent

```bash
python tools/Validators/ValidateEpistemicDiscipline.py
python tools/Validators/ValidateEpistemicDiscipline.py path/to/module.mrs
python tools/Validators/ValidateEpistemicDiscipline.py --strict
cmake --build build --target validate-epistemic-discipline
cmake --build build --target verify-clay-dossier
```

## Policy docs

- `docs/Analysis/Discipline.md` — pattern catalog with DO / DO NOT tables
- `docs/Analysis/MarshalDefinition.md` — what Marshal is / is not; `referee_class` tiers
- `AGENTS.md` — MRS-only closure mission
- `.cursor/skills/marshal-epistemic-rigor/SKILL.md` — skill for proof edits
