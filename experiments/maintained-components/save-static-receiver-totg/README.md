# Static receiver validator and ToTG hub doors

These are source-only follow-up components for the experimental EUR v0 practice build. They do not form a complete RPX, installer, or working Save/Restore implementation.

The renderer preflight accepts one exact original static receiver in a packet queue only after its published pointer, 32-bit constructor guard, and derived vtable match. The same object remains excluded from child-list/queue ownership. Existing two-pass observation and disjointness checks remain in place. This repairs a reproduced validator gap; attribution to the user's most recent manual Save failure remains unproven.

The ToTG adapters remove the Command Melody pedestal condition only for the exact west/north hub door actors when Rando Mode is enabled in Siren. They preserve servant checks, normal mode, actor data and the separate upward-beam condition. The stage+4 relocation uses a complete HA/LO pair.

`build.py --compiler PATH --output NEW_DIRECTORY` rebuilds the two relocatable objects with the pinned devkitPPC compiler/linker. No game dump, SDK binary or recovery object is used. All engine/metadata interfaces remain undefined. The local compiler's standard headers remain external dependencies. A complete image needs reviewed typed Native bindings, hooks, metadata and lifecycle integration. No hardware success is implied.
