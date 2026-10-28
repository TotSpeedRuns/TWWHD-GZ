# Saved-data validation performance

This is an authored partial source snapshot from the post-v148 private branch.
It is not a standalone library, a complete current build export, or a released
Wii U update. The reproducible integrated export remains **v126**. No game
instruction blocks, assets, binary image, captured state, private profile or
raw diagnostic log is included.

The BOH bomb-restoration path previously hashed the same 934,136-byte saved
archive 244 times in the measured test. `source_bomb_scope.inc` wraps each
original whole BOH/BFM body once, checks the genuine current pin, owner,
request, frame and thread, and permits reuse only inside that fixed call.
Reentry, lost plans, recovery and conflicting WCT purpose refuse reuse.
Outside that purpose the original full checks remain.

The current six-kind saved-format memo is pin-bound. It records a result
only after the actual saved-data validator succeeds, then rechecks the real
digest and pin. No current-world validation result is cached. Before owner
publication, `source_bomb_final.inc` forces the existing full archive hash
between fresh owner/pin checks. The following publication tail remains
nonfallible private cleanup. See `integration-seams.json` for exact authored
changes and the external parent dependencies.

Three genuine Cemu bomb expiry/restores passed with fresh bomb identities and
exact saved fuse, bomb pose and Link pose, without loading a room. Covered full
archive hashes fell from 244 to 16. Observed restore intervals were
1.277 / 1.216 / 1.199 seconds; the preceding same-driver run measured
2.206 / 2.144 / 2.135 seconds. These host intervals include diagnostic reads
and are not a controlled CPU benchmark or Wii U performance measurement. A
separate ordinary WCT replay restored seven natively cut grass units. The
existing driver counted format invocations, not actual saved-format scans or
cache hits.

`identity_validate.c` uses an exact sorted PID set for larger detached identity
graphs, preserves row validation order and delegates edge checks to the
unchanged `_gz_identity_edges_safe`. It has no live reads, allocation, lock or
persistent cache. The small assembly shim preserves the old r6..r31 contract
while calling C. Its bounded automatic storage and full kernel/thread stack
headroom still require care. Emitted PPC tests show fewer instructions for
the real 158-row graph; the isolated earlier native test did not demonstrate
an end-to-end speed gain from this validator alone.

These changes do not authorize generic RAM replay or solve actors born after
capture, held equipment, death/void, different rooms, complete cutscenes or
music. All original route payloads and normal save files were preserved.

## Same-worker saved-format memo extension

The three authored `source_pin_format.inc`, `source_pin_format.h` and
`source_custodian_format_policy.h` files extend the existing eight-byte memo to
the actual preadoption SOURCE custodian. They add no DATA/BSS, native hooks or
native import sites. A real READY/admission or ADMITTED/install borrower must
recheck the complete sealed 76-byte current custody, actual pin, source/used,
Row identity/generation/hash, frame/thread/worker context and current revision
before and after each operation. Only an actual complete saved-only validator
success can mint a bit. Pending custody with a lost purpose refuses; unissued
SO decoding without custody remains FULL. All CURRENT comparisons, original
ordered work callbacks, full admission/publication and final hashes remain.

`custodian-format-integration-pins.json` names the exact LF 805 SOURCE parent,
three forward/inverse-verified integration hunks, public dependency pins and
same-translation-unit order. Copy the scalar policy header into the original
SOURCE directory and apply `integration.patch.txt` only to its exact pinned
parent. The published complete header/include also replace the older public
four-kind snapshot; this patch is against the later six-kind 805 parent, not
against the older public file. Real SAND/CGA saved validators and private
owner/OSA/SOM integration remain external dependencies. No test substitutes
successful validator, ownership, pin or native API results for them.

The tested extension completed three ordinary WCT/SSP saved-holstered/current-
drawn sword restores, with exact pose/equipment and per-restore journal/hold
cleanup. In that ordinary lane, CGA wrappers stayed 9 and complete saved-leaf
entries changed 9 to 7 relative to the preceding finite campaign. SAND stayed
250 wrappers/1 complete leaf; covered SOURCE FNV evaluations stayed 7.
A separate Debug+Input (flags 3) campaign completed three natural-expiry bomb
factory recreations with exact fuse/pose and fresh identities; ordinary WCT
was unchanged. Its SAND 237/9, CGA 11/10 and covered FNV 16 stayed unchanged.
Stable post-bomb helper state is only a post-campaign observation.

These are distinct finite Cemu scopes, not full-world replay or a hardware,
latency, total-hash, bytes-read, scan-avoidance or comparative speed claim.
`custodian_format_cases.c` tests only actual scalar predicates.
Source-contract tests check the retained real validators and pre/post guards;
neither test supplies positive native/lease outcomes. The complete integrated
public builder remains **v126**. No game bytes, captures, profiles, private
paths, host memory reader or binary object is part of this source projection.
