# Source and build status

The [common C admission primitive](../experiments/maintained-components/common-admission/README.md)
is now published as an exact reviewed standalone source component. One private
Cemu test exercised four real OS threads on guest cores 0, 1 and 2, with nested
and overlapping shared work, blocked wakeup, two-Gate ownership checks and clean
termination/join. It is not an actual game-writer/lifetime barrier, memory
snapshot implementation or Wii U qualification. No proprietary bytes or private
runtime reports are included, and the full integrated export remains v126.

The full reproducible export remains v126. The maintained snapshot includes
UI components through R724 and save-state components through R703. The reviewed
v142 component update is described below; its complete multi-object build has
not been exported here. This snapshot does not reproduce the complete v142
release or establish full-game coverage.
R724 UI testing does not promote the older save-state sources. A separate
post-v140 restore-performance folder now supplies the reviewed authored READ
scope/receipt includes and private collider metadata validator. It does not
export a complete newer restore build. Three expired-bomb and three idle Cemu
restores passed; native Wii U, post-void and sword-model gaps remain unresolved.
See the maintained restore-performance compatibility notes for exact limits.

The v140 source proposal additionally stages the reviewed bottle editor and
Frame Advance air-handler component. It does not replace the v126 full
builder or export the full newer runtime. One Cemu normal-swimming air test
passed pause stability, three one-point steps and ordinary Resume draining;
diving, drowning, every area and Wii U remain unqualified. The 88-byte
retail profile is absent from source and must be extracted locally from
the exact owned EUR v0 input. See the maintained-component compatibility
notes. This source snapshot does not reproduce the full release or claim native
Wii U acceptance.

The post-v140 UI update publishes the relocation-aware Forest Water guard,
Inventory stick cursor component and the menu ownership gate for Debug/Input
text. The Forest Water guard still checks all 22 native entry words exactly;
it recomputes only the seven documented DATA relocation halfwords from the
existing native heap-slot alias. The 88-byte retail signature remains a
generated local-only input. No raw game code or live diagnostics are exported.
Focused Cemu checks passed all four Forest Water slots, its shared countdown,
native Plus save/restoration, stick navigation and complete Tools toggle text.
Wii U acceptance of these post-v140 changes remains pending. These files do
not claim to fix the remaining MSS typing or save-state coverage failures.

The maintained location-warp update adds an authored table generator and
compiler driver plus normalized destination, repair and location metadata.
It includes attribution to the requested Teotia reference and an independent
metadata-only check. Original room bytes and private diagnostics are absent.
The generator depends on the private qualified R755 base and matching
component objects; it cannot reproduce the current release from the exported
v126 template. These are raw warps, not new route/memfile payloads. Full
current-runtime export and broad native/hardware coverage remain separate.

The v142 maintained component adds the authored practice-travel gear leaf,
strict source applier and three Ganondorf entrance-word corrections.
Cemu observations identify the retained Zelda conversation and native
controls, and demonstrate a naturally raised hide bit followed by raw
and memfile recovery. No pixels or Wii U hardware were observed. No
save-state coverage, persistent slots or full current build is added.
See [the component's exact scope and pins](../experiments/maintained-components/gear-travel/README.md).

The static Triforce component adds the authored RGBA compositor, menu
composition and small ABI bridges, with a standalone compositor smoke test.
The additional full-mask draw policy uses a separate full-image descriptor when
all eight ownership bits are set, then draws the large-page selection outline.
Owned sprites, asset assembly and complete current build dependencies remain
excluded. The R769 baseline passed all 16 real Cemu shard give/remove operations
and navigation. After pure emitted-code and positive-pixel checks, the integrated
R771 full-mask image passed 16 give/remove operations and CPU-canvas comparisons
of 921,599 large-page pixels plus 4,500 Inventory-preview pixels in one isolated
Cemu session. The large comparison excludes the sole authenticated transparent
refresh pixel at (0,0). Final TV/GamePad and native-font composition, Wii U visual
acceptance and native shine remain unverified. This partial source does not replace the
reproducible v126 export or add Puppet Ganon/cutscene save-state replay.
See [graphic component scope](../experiments/maintained-components/triforce-graphic/README.md).

The maintained generic CPU observation experiment adds reviewed authored
collector/header, guarded root/OIS includes, small ABI/input bridges and
interface headers from the private R787 component. Its two-class full CPU-byte
capture is diagnostic only: mask63 keeps every unresolved closure bit, and no
native capture, world freeze or replay is claimed here. Matching providers,
link inputs and complete current integration remain unexported. The maintained
MSS four-file snapshot now follows explicit Plus/stick/Plus, Plus/Plus/stick,
and Plus/B/stick order, with the last completed display retained during a new
pause. Pause duration is diagnostic only. Twelve sequences at three dwell
values passed in one private Cemu run; Wii U chronology and pixel output remain
unqualified. See the maintained MSS rules and portable classifier tests.
The MSS ordering/latch snapshot is first delivered in v148. The full
reproducible builder remains v126; these partial snapshots do not constitute a
complete newer build.

The large legacy assembly template mixes authored patch code, compiler-generated
code and generated tables. It is not a good canonical review surface for the
whole project. The newer maintained C modules do not remove that limitation.

## Architecture cleanup requirements

- Keep authored C/C++ sources separate from generated assembly and data.
- Keep assembly bridges limited to documented patch/ABI boundaries. Record the
  original instruction, required registers, entry/return contract and test for
  each retained bridge. Existing hooks are not all justified merely by being
  present in the current build.
- Use readable modules, common integer/layout types, named fields and explicit
  size/offset assertions. Document alignment and section attributes individually;
  do not retain unexplained attributes as decoration.
- Compile ordinary logic through a dependency-aware build. Python remains useful
  for RPX transformation, relocation checks and asset extraction; a Python driver
  is not itself a defect. Version-specific source rewrites and machine-local
  build copies need replacement with explicit build inputs.
- Use verified typed interfaces for existing game objects. Choosing C++ does not
  by itself establish a native class layout, ABI or resource lifetime.
- Export enough source, toolchain information and lawful local-asset extraction
  instructions to reproduce the current release. Preserve native regression
  evidence when changing the build or representation.

These are open requirements, not claims that a language migration or a current
full-release export has already been completed.

## Dependency provenance

The build uses the public devkitPro toolchain and WUT interfaces. For example,
`GX2DrawDone` is declared in WUT's public `include/gx2/event.h`; knowledge of that
API alone does not establish use of a leaked SDK. WUT publishes a zlib license.
See its [source](https://github.com/devkitPro/wut) and
[license](https://github.com/devkitPro/wut/blob/master/LICENSE.txt).

This identifies a public dependency; it is not a blanket provenance certification
for every project file or for model training data. A complete dependency/header
provenance inventory remains to be finished. Existing third-party source notices
are listed in [NOTICE.md](../NOTICE.md). Proprietary SDK headers and game assets
must not be added to the published source tree.

The maintained whole-frame observer and detached registry capsule are authored
partial components. One private Cemu test observed real frame returns during
gameplay and menus; its read/ownership functions remain false. The registry
component validates detached ordered graphs and identities only, with no live
provider, replay or readiness grant. Worker/resource/writer exclusion and real
independent undo remain open. These files do not make a newer coherent build
export or qualify full-game save states. See the two component scope documents.

The maintained Delivery Bag source adds physical-copy counts, Left/Right
remove/add and the existing A toggle through the existing guarded inventory
dispatcher. One private Cemu cohort completed21 Delivery cases and two neighbor
bag checks. The first driver remains failed after a wrong Spoils selector; a
separately reviewed same-process remainder completed those two checks. Five
unassigned saved bindings/cache bytes and all six normal files were unchanged.
Actual equipped remapping, native letter use and WiiU remain untested; bag
acquisition grants remained in disposable RAM. No full-RAM, save-state or
complete current-build claim is made. See delivery-copies/README.md in the
maintained components. The reproducible integrated export remains v126.

## v147 partial heap and SOURCE modules

The [v147 maintained snapshot](../experiments/maintained-components/native-heap-v147/README.md)
publishes reviewed authored C, headers and includes for worker admission,
HG-first allocation metadata censuses, genuine process-arena birth provenance,
and SOURCE digest/format reuse. Its combined Cemu observation covers two usable
metadata cuts and one capture/three idle typed WCT restores. It does not establish
opaque heap capture, physical lifetime/resource/device closure, universal replay
or Wii U reliability. GPA's boot report is historical; its matcher was not
exercised in this campaign. The default-disabled raw-copy proposal is excluded.
Native wrappers, complete owner implementations and the current multi-object
builder remain external dependencies. The reproducible full export stays v126.

The v148 partial source update adds the authored CM row-cache, SDK empty-sweep
rule and exact HL existing-key lookup functions with portable checks. It keeps
the prior v147 observation/admission work and adds the MSS ordering/latch fix
first delivered in v148. Three quiet
bomb expiry restores and a bounded ocean-streaming Cemu campaign passed; Wii U
lag/crash improvement remains unverified. No triangular-probing experiment,
raw-copy kernel or new WORLD/READ permission is included. See
[streaming bookkeeping scope](../experiments/maintained-components/streaming-bookkeeping-v148/README.md).

## Post-v148 save-state performance source

The [saved-data performance snapshot](../experiments/maintained-components/save-state-performance/README.md) adds genuine BOH pin-scoped source reuse and the pure identity validator. Three Cemu bomb expiry/restores and a separate seven-cut foliage replay passed. Observed restore intervals fell from roughly 2.14 seconds to 1.21 seconds in the same driver; this is not a controlled or hardware benchmark. It is a private-branch partial snapshot, not a new released build or full-game acceptance. Integrated reproducible source remains v126.

## Post-v148 fixed root borrowing

The [fixed root-borrow snapshot](../experiments/maintained-components/wct-root-borrow/README.md)
publishes authored CLOCK/PERSIST getter fragments, the real coupled WHL/job
provider and exact integration seams. It preserves ordinary independent
admission outside the genuine retained transaction and refuses lost borrowers
without fallback. Three private Cemu sword transitions and three separate
standalone bomb expiry/restores passed. These finite observations do not
establish Wii U, broad actor retirement or full-game coverage. This is partial
experimental source; the reproducible integrated export remains v126.

The same-worker SOURCE custodian snapshot is an additional partial authored component. Its fixed callback repair forwards the real slot, MCA and bflower callbacks under one existing acquisition; private Cemu sword tests cover three restores. A separate three-case factory bomb recreation regression also passed; that lane did not participate in WCT or BMH services and supplies no bomb speedup claim. Complete integrated reproduction remains v126; no Wii U, full-game, release or speedup claim is added.
