# R35 full-particle native Save source overlay

This reviewed overlay adds the authored whole-particle Save increment to
[native-save-v154](../native-save-v154/README.md). It contains nine replacement
files and four new files, reusing the other 177 v154 source files. The relative
manifest verifies reconstruction of the complete 190-file R35 source tree.
Seven accompanying JEL/assembly-hook files supply the actual maintained
constructor-receipt producer; the consumer layout header appears once, in the
overlay. No duplicate copy of the unchanged 177 files is included.

The complete reproducible integrated builder remains **v126**. This is a partial
source component, not a standalone build or a complete current release export.
Matching native wrappers, imports, other maintained providers, exact linking and
packing rules, and lawful local game inputs remain necessary.

## Apply and integration

Start with `native-save-v154/source/`, then copy `overlay/` over that source tree.
The result has exactly 190 files. `producer/jel/` contains the complete authored
JEL/JEX metadata producer and its headers/includes; `producer/hooks/` contains
the matching assembly bridges. Compile these as their separate native objects
with the existing soft-float ABI. The selected partial-link recipe preserves
the `_gz_jel_birth` to `_gz_lsc_jel_birth_previous` symbol rename and the matching
external provider joins; these files alone do not supply that full builder.

Keep the v154 CODE alias definitions unchanged. Add
`native-data-aliases-extra.ld` to its DATA bindings: the three declarations name
original-game control/manager/seed DATA slots, not captured pointer values.
Their loader relocations and the native branch/return bindings must survive
partial linking and final packing. No game executable bytes are supplied.

The auxiliary constructor receipt is a separate 256-byte array. Existing
JEL state 10880 and emitter-pool receipts 256 retain their layouts. The completion
hook observes the actual constructor's manager, auxiliary one-past pointer and
loop count, including null-allocation and zero-count handling. It does not
manufacture a constructor, heap generation, pointer lease or replay permission.

## Observed Save and limits

One closed Cemu session completed **one Save and zero Restores**, with a paused
placed bomb in Tower of the Gods, room 2. Two complete 14,701,444-byte private
archive readbacks agreed. The 3,838,400-byte JPCv2 packet includes the complete
5,770,168-byte manager allocation, emitter array 153,008 bytes, particle
array 888,008 bytes and auxiliary array 31,208 bytes. The 16,804-byte control is
already covered by the outer heap and is not duplicated. All five current
allocation bindings in that case legitimately had heap mutation epoch 0.

The measured native Save operation took 3.328 seconds in Cemu. This is one scoped
capture observation, not Wii U timing, a general performance guarantee or
actual-console memory-fit proof. The normal profile and six save files were
restored after the session, and the engine was closed.

Heap mutation epoch 0 is valid when it equals the actual current binding; the
allocation and heap generations and current-binding checks remain required.
Child edges may retain historical generations after removal or slot reuse.
Such bytes are preserved as inert data; they cannot revive a generation or
authorize a pointer read. Private emitted-code tests cover that history
contract. The native Save case had no nonempty historical child edge.

JPC remains **version 2, missing 255**. Particle replay and lifetime adoption are
not implemented. The separate heap Restore experiment does not restore these
external pools. Resource/device/indirect-writer closure, Wii U operation,
actual-console memory fit and full-game Restore remain unverified. The earlier
bomb-smoke Restore failure remains a limitation. Future fence/owner-export work
is excluded from this tested overlay.

## Inventory

Only authored text source, assembly bridges and static symbol bindings are
included. Private handoffs, reports, captures, machine paths, profiles, runtime
pointer values, game assets/executables, SDK binaries and keys are excluded.

`export-manifest.json` lists raw hashes of every component file except itself,
and all 190 reconstructed source hashes. Narrow Git attributes preserve exact
authored source bytes. From the repository root, verify with Python:

```python
from pathlib import Path
import hashlib, json

root = Path("experiments/maintained-components/native-particle-save-r35")
manifest = json.loads((root / "export-manifest.json").read_text())
actual = sorted(p.relative_to(root).as_posix() for p in root.rglob("*")
                if p.is_file() and p.name != "export-manifest.json")
assert [r["path"] for r in manifest["files"]] == actual
for row in manifest["files"]:
    assert hashlib.sha256((root / row["path"]).read_bytes()).hexdigest() == row["sha256"]
base = root.parent / "native-save-v154" / "source"
for row in manifest["reconstructed_source"]:
    source = root / "overlay" / row["path"] if row["overlay"] else base / row["path"]
    assert hashlib.sha256(source.read_bytes()).hexdigest() == row["sha256"]
assert len(manifest["reconstructed_source"]) == 190
```
