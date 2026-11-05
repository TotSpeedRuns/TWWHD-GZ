# Bounded Setup175 profile recovery

This package preserves authored integration source and numbering metadata for
Setup175/Game175. It applies a small same-width profile to an exact owned
Setup174 executable and creates the exact Game175 installer recipes from owned
original EUR v0 and Game175 files. It is not a full Setup or game source build.
No compiled files, game/SDK bytes, recipes or private filesystem paths are
distributed here. The included MIT license covers authored code and metadata.

Python3.10+ is required. Supply the five-file Setup174 app directory and the
owned game files explicitly; no recovery folder layout or compiler is assumed:

```powershell
python -B tools/build_setup175_profile.py --repository C:/path/to/TWWHD-GZ --setup174 C:/path/to/owned/Setup174 --game-rpx C:/path/to/owned/Game175/cking.rpx --game-title C:/path/to/owned/Game175/Title_00.szs --original-rpx C:/path/to/owned/EUR-v0/cking.rpx --original-title C:/path/to/owned/EUR-v0/Title_00.szs --output C:/path/to/new-output
```

Output must not exist and must be outside source/repository/Setup input trees.
The tool verifies all owned hashes and both exact recipe round trips before
writing. The resulting root RPX/WUHB/recipe hashes must match `profile.json`.
Its output is private game-derived material and must not be added to the source
repository. It performs no upload, console transfer, install or ZIP packaging.

## Source dependencies and gaps

The two external parser files are verified byte-for-byte in published commit
`db12c90fed8f0141c6625b4a162a27581c212a22`: `patcher/native_rpx.py` and
`patcher/title_engine.py`. Their Git blob SHA1, SHA256 and sizes are pinned in
`profile.json`. These files must be supplied through `--repository`.

The older local `build_setup_profile.py` and `build_setup_recipes.py` helpers
were absent from that published tree. This export includes only the authored
WUHB parser and eight used recipe encoder/decoder helpers as small new modules;
it does not claim those old full helper files were already public. Verified
source bytes are compiled directly, avoiding incidental Python bytecode caches.

The exact owned Setup174 binary remains a required input. Its complete Native
C++ source is not recovered by this export. Game175 also remains an exact owned
target input: the separately published algorithms do not by themselves provide
a complete current game builder. No SDK/toolchain or original game is bundled.

## Typed profile contract

All offsets are preferred addresses in the exact Setup174 parent, not portable
runtime pointers. The current175/prior173 digest strings and transaction names
remain in original DATA-owned literals; the existing cleanup current-digest
arrays are32-byte immutable CODE-domain objects, authenticated by their typed
definitions and original relocation bindings during private qualification.
The public config records object names/addresses/extents rather than bytes.
Do not reuse this map against another Setup build merely because labels match.

The Native revision remains exactly25 printable bytes:
`v175 USB Test / Game v175`. The compiled receipt length is unchanged. The
five selected-device/role hooks, every existing loader row,17 historical roles,
imports, FileInfo and cleanup165 policy remain exact. Only current/profile
identities, same-width labels and their CRC values change. Original Native
strict-current, unknown-file and title stage/backup checks remain authoritative.

The bounded profile builder verifies source inputs and outputs; it is not a
replacement for the serialized CPU/independent review receipts. Physical USB
repair, menu/SaveRestore behavior and loading latency remain hardware tests.
