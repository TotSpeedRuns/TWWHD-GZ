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
The A-button recovery in v125 still has a reported console failure under
investigation; the reproducible build does not imply that bug is fixed.

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
