# Build status

The standalone native runtime build has been reproduced from this asset-free
source export. Local game artwork is required at build time.

| v125 output | SHA-256 |
| --- | --- |
| RPX | `3d177054288b3a695c3f2feba8f62ffbca2b9912f51a58098a43964398ef90b5` |
| ELF | `b3b6e1ad6227083519d232407c650d38560c75dbe3ff6e51d92c43e6069776ae` |

The shipped v125 candidate passed 346 scoped compiled-code checks in the local
development harness. That larger harness has not yet been made portable here.
The tests in this repository cover the source/asset publication boundary only.
The v125 console failure was traced to queued actions not being dispatched.
The v126 candidate adds 298 missing C-object address fixups and rejects omitted
linked fixups during packaging. Twelve additional moved-DATA PPC checks pass;
the matching runtime RPX is
`52f36256f3e89c3f0bce2e0a8416477e61e2ecde27ae408258593bdfc3785918`.
The compiled ELF and gameplay data are unchanged from v125. Console acceptance
is still pending; this does not claim the user's failure is hardware-verified fixed.

## GZ Setup and desktop patcher

Their source is included for review. Full release packaging additionally needs
devkitPro/WUT, libmocha and generated patch/repair recipes from the owned input
game. The existing machine-specific release packager is not included. A portable
end-to-end Setup/desktop packaging command remains to be added; `build.py` builds
the game runtime and patch payload, not the installer application.

## Experiments

The experimental save-state components are source snapshots with separate
integration requirements. The 202 private integration checks cited in their
README are scoped evidence, not a public full-game test suite.
