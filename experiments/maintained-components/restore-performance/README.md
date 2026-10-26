# Post-v140 restore-performance components

This is an authored source snapshot from the privately tested restore path. It
does not replace the older R703 save-state snapshot or the complete reproducible
v126 runtime export. It is not a complete current-release build or a standalone
library. No native game code, artwork, runtime assembly aggregate, binary build
output, saved state or diagnostic dump is included.

## Saved archive validation during five-view queries

`factory_read_scope.h`, `factory_read_scope.inc` and
`factory_source_receipt.inc` add one genuine native READ hold around the existing
five mapped-view queries for a restore service. The private purpose binds the
current request, token, frame, epoch, thread, service, progress and source-owner
identity. Its metadata callback does not follow actor, source or resource bytes.

The separate receipt reuses only the saved archive validation result and nine
saved scalar fields inside that hold. The current working pointer is derived
again. Full original archive verification runs at entry and exit with reuse
disabled. Current working-copy comparisons, source-bridge, process, collider,
heap, generation, topology and native binding checks still run. An unrelated
error or poisoned receipt does not hide exit verification while its physical
key remains current; a revoked key is not followed. The first error survives
cleanup. The READ purpose is retired before typed native writers. Recovery
retains the existing source-free before-image path.

## Fixed private collider observation table

`members.c`, `members.h`, `process.h` and `observer.h` contain the small authored
CM observer and its required metadata declarations. The sole new predicate is
`object | generation` instead of `object || generation` when both unsigned
fields of an EMPTY row must be zero. A corrupt nonzero object also causes a read
of its generation field, in the same mapped private row, before refusal.

All 8,192 row checks, exact active count, locks, corruption handling and the
complete retirement/query paths remain. These are private observation-table
writes; scalar game addresses are never followed by this validator. A matching
observer row is not native lifetime or restore authority.

The actual compiled operation-count comparison saves two instructions per
successful EMPTY row; tested dense LIVE, DEAD, mixed and nonoverlap paths have
unchanged instruction and table-read counts. The validator still scans the
whole table. This is not a Wii U frame-rate or choppiness fix claim.

## Integration contract

`integration-seams.json` records exact authored before/after hunks and full-file
hashes; `integration-seams.txt` is their readable unified diff. They describe the
owner archive lookup, mapped five-view loop, SRH metadata callback and CM EMPTY
predicate only. Refuse a mismatched parent. Do not replace a whole runtime with
an old generated aggregate.

The factory includes require the existing integer types, owner/source receipt
types, archive validator, source read lease, HG/SRH gate, BMH/BOH/HBP progress
metadata, native-thread accessor and their original APIs. Those dependencies
are not all exported here. Include the scope before the mapped-view consumer
and the saved-source receipt alongside the original owner source receipt.
`process.h` and `observer.h` declare existing observation services; their
implementations and qualified native hook/ABI boundaries are separate inputs.
There are no new game hooks, snapshot fields or heap reservation requests.

The source was built with the existing devkitPPC 750 configuration and current
symbol/relocation integration. This folder supplies reviewable source, not a
portable command that reproduces the newer multi-object runtime. Existing UI,
air, inventory, Movement and MSS components must remain current when integrating.

## Qualification limits

One controlled Cemu session passed three expired placed-bomb restores at
Outset and three idle restores in the Sickass Skip setup. The bomb cases kept
the saved pose and fuse, produced new native actor IDs, did not load a room,
and preserved source and normal-save hashes. Each bomb restore recorded seven
scope entries, 105 saved-result uses, seven exits and seven full exit
verifications with no reported receipt error; purposes and owned gates cleared.
Idle restore acceptance does not qualify the missing sword-model recreation or
post-void restore path.

The bomb wall times were approximately 2.19, 2.14 and 2.13 seconds; idle restores
were approximately 1.33, 1.30 and 1.30 seconds. The older two-run comparison used
the same source size and owner/contact counts but different capture bytes,
fuse and session history. These are descriptive combined-build observations,
not a causal component benchmark or hardware performance result.

Offline qualification covered 1,636 factory metadata/projection/refusal checks
and 218 CM differential metadata cases at two placements. Factory ACTIVE/HG
entry and intact-key exit stopped at real native boundaries in the offline
tests; their results were not supplied by API doubles. Genuine success was
observed only in the scoped Cemu cases above. Failure/lease-loss inverses,
post-void and sword dependencies, broader actors/areas/bosses/cutscenes,
cross-area behavior and Wii U performance remain unqualified.
