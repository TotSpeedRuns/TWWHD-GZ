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

The existing four-kind saved-format memo is pin-bound. It records a result
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
