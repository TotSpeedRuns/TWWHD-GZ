Experimental clean-abort transaction retirement

A restore refused before native apply could leave a sealed, already-ended
heap transaction with its prepared private partition records still present.
The final emptiness check then refused cleanup, and the caller skipped the
leaf retirement that would have made that same check succeed. Later captures
mistook those stale records for a live restore and refused again.

The change invokes the existing token-checked finish-leaf routine before the
final private-data clear and emptiness check. It keeps every existing owner,
phase, source, pin, native-job and physical-token check. A foreign, live,
failed or incomplete transaction is not cleared. This is not a force reset,
new actor permission, or new reconstruction method.

The actual PowerPC component reproduced its previous whole-object baseline.
Independent emitted review found only the cleanup function changed: 72 bytes
of additional code, with no new saved-state storage. The integrated private
image retained the other 79 component inputs and all 803 checkpoint payloads.

An ordinary Cemu capture and subsequent restore with a newly drawn sword
succeeded, but did not trigger the intended clean refusal. That test is
inconclusive for this specific cleanup branch. A natural effect-mismatch
regression is still pending. No Wii U verification or full-game support is
claimed, and this source snapshot does not ship a new console update.

The original and candidate files plus the patch contain authored project
source only. They depend on the other transaction participants and are not
a standalone executable or a complete current-release build export. The
repository's integrated reproducible builder remains v126. Game binaries,
memory captures, personal profiles, saves and native diagnostic dumps are
excluded.

Later bounded verification (2026-10-29): the retained cleanup path was exercised
by one natural quiet heart and a separate two-green-rupee cohort. Both completed
the typed inverse and native retirement, cleared the checked private records,
then committed a separate manual Restore. See the quiet-heart0-lifecycle source
snapshot for exact scope. The original effect-mismatch case, arbitrary actors,
one-action completion and Wii U behavior remain unqualified.
