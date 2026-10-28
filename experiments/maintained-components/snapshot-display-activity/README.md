# Snapshot display activity policy

This partial authored component lets ordinary snapshot admission retain the
Debug and Input Viewer display flags. The complete reproducible integrated
export remains **v126**; this is not a standalone current-runtime builder.

`snapshot_display_activity.h` classifies activity without clearing user flags
or issuing an owner, read lease, renderer token or successful API result.
The seven hunks in `integration.patch.txt` update four authored C files: the bomb
chooser, no-bomb chooser, bomb context and owner admission/diagnostic checks.
Only display bits 1 and 2 become eligible. Existing Upcharge admission remains
only where it was already allowed; every other gameplay activity restriction
and all SOURCE, CURRENT, resource and lifetime checks remain.

The existing integration forces menu1 before the queued worker. Its normal
HUD dispatch and both fast HUD paths suppress Debug/Input drawing while that
menu is active. The genuine owner also retains native framework-draw and input
suppression. These required runtime/owner dependencies are not replaced by
this header, and the display policy is not GPU, OS or generic-memory authority.
Private death probes, generic RAW paths, frame advance and MSS are unchanged.

One private Cemu campaign enabled both displays through the normal Tools menu,
captured a placed bomb, then completed three natural-expiry restores with the
expected fuse and pose and unchanged source/save files. A separate campaign
completed three saved-holstered/current-drawn sword restores with both displays
enabled. It covers that finite stow direction, not the opposite constructor
direction. An initial harness argument-count error stopped before capture;
the corrected harness completed all three restores. That initial stop was not
a GZ product restore failure. No full-game coverage, Wii U qualification,
delivery approval or comparative speed improvement is claimed here.

Apply the patch only to the authored parent-source hashes in `source-pins.json`,
after projecting UTF-8 source to LF. Copy the shared header into both listed
translation-unit roots. Parent/final raw and LF hashes are recorded separately;
the complete parent translation units and private current builder are omitted.
No game method bodies, native instructions/assets, binaries, captures, saves,
private profiles or host-memory tooling are included.
