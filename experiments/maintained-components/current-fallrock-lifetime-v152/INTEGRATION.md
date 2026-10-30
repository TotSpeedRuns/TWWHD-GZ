# Exact predecessor and native integration

`baseline/lifecycle_intent.inc` is the exact authored predecessor of this delta.
Apply `integration.patch` only to that matching source, producing the two files
in `source/`. Use the explicit SHA-256 identities in `source-pins.json`; do not
apply it fuzzily to the older integrated v126 or earlier lifecycle snapshot.

The new include is inserted after `lrr_empty_root` in the existing translation
unit. It deliberately uses the enclosing project's exact `LrrActor`, `lrr`,
`lrr_binding`, process/contact records, span/read macros and existing helpers.
It is not a separately compilable generic actor API. The same full parent
component was reproduced before the new component was compiled.

Required genuine dependencies are:

- Current process/collider metadata and fresh native allocation bindings from
  the existing PO/CO/HL/OB owners; `_gz_ob_try_capture` and validate must be real.
- The real guarded model/solid identity provider (`_gz_gh_solid` and its exact
  identity record), live layer resolver and current job/heap ownership scope.
- Correct native TEXT/DATA bases and existing resource-control/vtable aliases.
  `_gz_brg_native_resource_control_slot` and `_gz_pr_resource_control` must join
  the actual same native resident-resource control. No stub may stand in for it.
- Existing native process deletion, LRW physical writer and genuine current
  admission. The ordinary engine runs its own destructor and resource release.
- Existing CTP inverse, WCC original commit blocker, sealed logical Source
  reservation and continuation dispatcher. Retirement completion is not success
  until a fresh ordinary restore commits all original participants.

The resource check follows only the exact current resident Always entry and
its model index32. Every dynamic envelope receives fresh real allocation
qualification and owning edges are reread. It neither writes resource
reference counts nor restores saved native pointers. Native methods and
profile/body offsets are code integration constants, not captured native bytes.

The emitted comparison independently retained the original profile 255 loads,
failure edges, calls, retirement/context paths and unchanged other component
bodies. This export includes no proprietary disassembly or native object.
Native cancellation, timeout and mixed-cohort behavior are not exhaustively
tested. Do not remove a guard to admit another class; establish that class's
actual lifetime/resource closure first.
