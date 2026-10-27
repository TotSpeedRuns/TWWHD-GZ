# Generic CPU observation experiment

This is reviewed authored component source from private R787. It is a bounded
read-only diagnostic, with no actor-type body or animation serializer. It
selects the first current live pot (profile453) and stone (profile454), copies
each complete owned actor allocation and childless Solid CPU model-heap span,
then compares the bytes again. The fixed private buffer is 64 KiB.

The existing native counter-calc callback supplies observation provenance.
It does not prove completed-frame execution, a world freeze, or atomicity.
All six unresolved closure bits (mask63) remain even when both reads agree.
The immutable OIS type3 image is diagnostic evidence; there is no replay,
save-state admission, or game-memory apply API in this component.

`generic_root_access.inc` and `ois_cpu_diagnostic.inc` belong inside their
matching owner translation units. The accompanying relative integration
hunks document reciprocal exclusions, the explicit JPG mode3 discriminator,
the private slot9 owner, and the two existing runtime seams. The headers
describe required PO/HL/GH providers, not replacements for those providers.
The small assembly wrappers retain the native callback/input ABI; they are
not general native authority or a new retail hook.

The independent source review and exact parent/component compilation were
completed privately. This source export does not claim a native capture,
gameplay rewind, full-game support, or a complete reproducible R787 build.
The full reproducible integrated export remains v126. No game bytes, assets,
raw user data, native objects, or private reports are included here.

`source-pins.json` records LF-normalized authored source parity.
`scope.json` and `integration-seams.json` describe the deliberate limits.
