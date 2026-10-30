# Same-call issued Source metadata tuple (v152)

This small authored patch is the selected v152 runtime's metadata query change.
It replaces two adjacent external getter sequences with an existing aggregate
Source API where that API already reports the exact issued tuple. Source645
itself is unchanged. The complete reproducible integrated builder remains v126;
this patch requires the matching current private world/bridge integration.

It accepts only the upper 32-bit `GZ_DIGEST_ISSUED` state. A zero low hash does
not mean refusal. Actual native ownership, current frame and registry revision
are checked before and after the same nonyielding call. FULL or REFUSE takes
the exact original five-metadata-accessor fallback; no result is retained across
calls. The separate pin-id comparison, original world/scope/owner/job/heap
checks and final forced source-body hash remain. It grants no body replay or
ownership capability. The early compatible registry-revision declaration is
part of this corrected patch; do not use the earlier missing-declaration draft.

The compiled current runtime passed one selected one-rock/one-contact Cemu
retirement/automatic-restore case, taking38.235 seconds. The class-only baseline
case took38.063 seconds. **No measured speed gain is claimed.** Static accessor
counts do not establish execution frequency or bytes hashed. An earlier broad
getter optimization was rejected after emitted code showed its presumed cost
savings were not real; that rejected version is not included here.

`portable_policy_cases.c` is the existing 28-case host policy fixture. It uses
explicit fake metadata returns solely to exercise the same decision function;
it is not a native provider, integration replacement, or save-state test.
The actual native tests use genuine providers. A host C compiler can build and
run that file directly. Its result gives no game authority.

Apply the patch only to the two exact predecessor hashes recorded in
`source-pins.json`. The patch contains the new function and both finite seams,
not the entire private world implementation. The existing verified-source-
digest ABI, real native owner, frame and registry providers, typed participants
and full integration/build machinery are external dependencies.
