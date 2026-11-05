# Recovered save-validation components

This folder preserves two authored PPC32 C providers. The NPS1 record validator
uses its existing borrowed-span interface; the saved-location formatter uses
four existing metadata objects. Neither provider defines new owners, globals
or SDK imports. They are component source, not a complete game or Setup build.

Rebuild the three qualified ET_REL objects with Python3.10+ and the pinned
devkitPPC r49 compiler (GCC15.2.0):

```powershell
python -B build_components.py --compiler C:/devkitPro/devkitPPC/bin/powerpc-eabi-gcc.exe --output C:/new-save-components
```

The output directory must be new and outside this folder. Source, specification,
compiler and final object hashes are checked. No original game, SDK library,
private recovery layout or repository parser is needed. Compiled objects are
generated locally and are not part of this source backup. The compiler's
support tools must be installed normally; the output hashes remain decisive.

The NPS provider has800 earlier finite native/C pairs and140 independent pairs.
Its ordinary same-name static join reduced one candidate's unresolved roles
from715 to714 without executing or selecting that candidate. All22 SDK imports
already had real definitions. Final code/data placement, loader/startup,
ownership and lifetime remain separate integration requirements.

The formatter recovers behavior already present in Game175. Its279 finite
paired cases cover borrowed metadata and a small original append suffix. The
detail fixture does not establish authentic whole-caller buffer boundaries.
The wrapper releases its lock before returning its cached descriptor, so
descriptor/module lifetime and exclusion of scratch/cache writers remain
required premises. No Source715 Mode0 return identity or owner alias is added.

`bindings.json` is the corrected local175 metadata map: `Unknown location`
occupies17 bytes including NUL. The original8-byte extent was rejected during
independent review and remains failed historical evidence in the private
archive. Only the metadata extent/description changed; formatter code and
Game175 did not. Preferred addresses are provenance, not portable pointers or
new Source OBJECT definitions. No native code/data excerpts are distributed.

The separate `tools/save-diagnostics175` folder provides the reviewed offline
JSONL reader. Its declared target, matching tuples and decoded source messages
do not authenticate a console, an atomic state, ownership, cause or success.
The historical source-map paths in its layout are hash provenance only; the
reader does not open them or import an operational collector.

This authored source and metadata use the repository's MIT license. Hardware
menu, Save/Restore, USB repair and loading performance remain unverified for175.
