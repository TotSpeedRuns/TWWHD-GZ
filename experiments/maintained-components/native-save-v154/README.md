# v154 native Save source component

This additive snapshot publishes 186 reviewed authored source files from the
maintained heap/save integration and its two native-address alias files. It
includes the v154 process-arena code-identity correction and existing on-screen
completion messages. It preserves the earlier maintained components and the
repository's older integrated runtime builder.

The complete reproducible integrated export remains **v126**. The newer UI,
lifecycle and Setup source records through v152 retain their stated scopes.
This directory supplies an additional v154 heap/save component; it does not
reproduce the complete v154 release or update the public UI/Setup build to v154.
No standalone build of this source snapshot has been verified.

## Change and integration contract

`source/native_lifetime/process_arena.c` compares native allocation, constructor
and release-site identities through 18 linker symbols. Their preferred native
CODE addresses are declared in `native-code-aliases.ld`; the existing DATA
bindings are in `native-data-aliases.ld`. These are authored address bindings,
not captured runtime pointers or an observed relocation bias.

The qualified integrated image retained 46 actual CODE operand relocations for
all 18 aliases. Each final loader target resolves through original executable
`.text` section 2. Object, linked ELF, payload and final RPX checks agreed, with
unique final fixups; arithmetic checks also retained comparison identity at
two synthetic TEXT load offsets. The existing 28 DATA aliases and 66 operand
fixups were checked separately. These are offline relocation checks.

The integrated correction adds zero permanent BSS bytes and zero snapshot-format
bytes. Existing ticket, native-thread, generation, ownership, birth and release
checks remain required. The correction does not grant a new allocation or
restore capability. The separate AS11 particle-capture increment is excluded.

The directories preserve their native integration boundaries:

- `source/native_lifetime/`: heap/allocation observation, process-arena identity,
  guarded heap capture/replay and format/metadata helpers.
- `source/gac/`: the retained game callback, capture coordinator and interfaces.
- `source/ois/`: owned-image storage, held views and source-origin integration.

Some laboratory source is retained behind its existing production feature
switches. File inclusion does not imply that every retained path is enabled in
the selected integration.

These modules depend on matching native wrappers, imports, admission/lifetime
providers, other maintained objects, menu/route integration and the lawful local
original-game inputs. The alias definitions and their operand relocations must
survive partial linking and final RPX packing. A preferred-address ABS definition
in a partial object is not a valid final loader target. This snapshot includes
neither the complete current multi-object builder nor those generated inputs.

## Observed result and limits

One controlled, closed Cemu session completed **one Save with a live bomb at
Outset and zero Restores**. The published saved archive was read and rechecked.
This supports that Save case only. No new Restore, replay, performance or
full-game correctness result is claimed.

An older v153 Cemu Restore test left bomb smoke afterward. That historical
failure remains a limitation; it is separate from the new Save-only result.
Wii U operation and actual saved-image memory fit are unverified. Full-game
save states, general particle/resource/device closure, repeated restores,
cross-area behavior and durable cross-launch slots remain unqualified.

## Public export boundary and verification

Only text source and the two alias definitions are exported. No private handoff,
runtime report, host path, profile, user pose, IP address, RAM capture, game
executable/asset, save, SDK binary or key is included. Native function addresses
in the authored bindings identify integration sites; they contain no game code.

`export-manifest.json` records component-relative paths and SHA-256 digests for
every exported file except the manifest itself, including this README. Paths
are sorted and contain no machine-local prefix. Source and alias bytes are
preserved exactly through Git by the component's narrow `.gitattributes` rules.
To verify the inventory from the repository root, run this Python code:

```python
from pathlib import Path
import hashlib
import json

root = Path("experiments/maintained-components/native-save-v154")
rows = json.loads((root / "export-manifest.json").read_text())["files"]
actual = sorted(p.relative_to(root).as_posix() for p in root.rglob("*")
                if p.is_file() and p.name != "export-manifest.json")
assert [row["path"] for row in rows] == actual
for row in rows:
    assert hashlib.sha256((root / row["path"]).read_bytes()).hexdigest() == row["sha256"]
```
