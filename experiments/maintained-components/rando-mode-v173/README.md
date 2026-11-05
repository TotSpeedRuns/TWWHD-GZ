# Authored practice components selected for the experimental v173 test

This versioned snapshot preserves the exact authored Rando Mode, indexed-renderer,
tracker R3, reservation, and diagnostic components selected in the v173 test. It
does not reproduce the complete native integration or promote reconstructed
Source715. The v172 menu-opening regression remains unresolved; v173 adds retained
input diagnostics and corrected tracker writer coverage. Hardware boot, menu,
Save/Restore, capacity, timing and full randomizer parity are not established.
All-items and the later physical-admission work are absent. Existing native/SDK
interfaces remain genuine unresolved requirements. The earlier rando-mode design
and renderer-normalization-v171 snapshots retain their original scopes.

Run `python -B tools/build_practice173_components.py --compiler PATH --sdk-include
PATH --output FRESH_OUTSIDE_REPOSITORY`. The byte-identical renderer171 sources,
strict driver and pinned public parsers are included in this source snapshot.
The five renderer173 files overlay a private build copy; no unpublished local
base is required.
The strict driver checks source manifests, complete compiler dependency sets,
compiler/tool/header pins, and exact whole ELF plus allocated ABI/relocation
identities. It compiles 27 selected asset-free objects without reading a game,
private object, recovery executable, or live capture. Toolchain binaries and WUT
headers are explicit external inputs and are not distributed. The reviewed
Windows compiler runtime itself is outside the claimed closure.

The capacity-text object additionally contains original localized messages.
Those messages and the generated capacity_policy.c are intentionally absent.
Use `python -B tools/generate_practice173_capacity.py --game-root OWNED_EUR_V0
--output FRESH_PRIVATE_DIRECTORY`, then pass `--capacity-source` with that output
to the component builder. Five exact owned EUR language packs are required; the
generator extracts only messages140/150, applies the authored capacity-tag recipe,
and writes game-derived output solely to the requested private directory. Do not
commit the generated directory. This optional path reproduces the28th object; it
does not create a game image or grant any runtime authority.

Build specifications and provenance list hashes and interfaces, not native
implementation bytes. No success providers or substitute ownership checks are
supplied. The pinned HD randomizer reference is credited in provenance.json and
UPSTREAM_LICENSE.txt; its USA addresses are not EUR binding authority.
