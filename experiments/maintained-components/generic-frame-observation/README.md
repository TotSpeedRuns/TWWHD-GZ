# Whole-frame return observer

This is an authored diagnostic component, not a save-state implementation or a
read permission. It observes the game's real whole-function frame entry and
return while always forwarding the original once. The wrapper preserves input
and actual native output CPU context, including paired FPR halves and user GQRs.
Its native entry is EUR v0 `0x02034FFC`; the displaced `mflr r0` must be replayed
before continuation `0x02035000`. Matching CODE aliases, relocations, current
thread getter, frame counter and a reviewed integration are required.

The fixed 128-byte report records entry/return frame, actual thread, opaque
instance and caller. Accept its core only with equal even publication sequence
reads before and after the report copy. Diagnostic counters outside that core
are best-effort. Pointer-like report values are labels, never followed here.
Both readiness/ownership functions always return zero. Native return alone does
not establish worker completion, no new submissions, external writer exclusion
or a current physical instance.

The private integration passed one controller-only Cemu session: 78 stable
reports across gameplay, GZ menu, native Plus menu and resumed gameplay. Each
record observed one existing frame-counter advance through the completed call;
normal and disposable save hashes were unchanged. This does not establish
cross-area/cutscene coverage, music restoration, arena replay or Wii U behavior.
The wrapper also has separate emitted CPU-context/refusal checks. No raw native
captures, linked objects or proprietary instructions are included here.

The full reproducible integrated export remains v126. These files alone do not
reproduce the newer private integration. The later job/FIFO experiment is not
part of this snapshot.
