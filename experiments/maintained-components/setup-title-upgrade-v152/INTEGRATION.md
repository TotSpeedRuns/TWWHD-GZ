# v152 title integration dependencies

Retain the five actual Setup title seams documented in the sibling
[`setup-title-upgrade-v151`](../setup-title-upgrade-v151/INTEGRATION.md)
component: shared header include, exact title hash, adapter include before
`ownedIdentity`, checked existing-file/repair recognition and normal final
title identity. The v152 adapter and selected hash replace only their known
v151 counterparts. Preserve normal RPX, icon, registration/content and volume
checks. Normal v152 main/revision binding is a separate release step.

The v152 selected title hash is
`4056ed52357e6391d31f3cae5d3d9159d73d3d06275a7c24bf2dc176cbf5294a`.
The known prior title hashes remain in the actual adapter; its recipe hash is
metadata for a locally generated original-dependent recipe, not an included
resource. Do not substitute a binary download or bypass original validation.

Copy the two exact `source/` files beside a matching current Setup `main.cpp`.
They require its real `Bytes`, workspace, cancellation, SHA-256, file reads,
restricted writes, logging, original-only `patchTitleArchive`,
`gzcopy::DirectFile`, filesystem, and `flushPracticeVolume` implementations.
The existing forward declaration and include order matter. No new SDK call
or receipt authority is added.

Native hardware install/repair/resume and power-loss behavior remain
unqualified. Do not interpret the 142-case host policy fixture or 10 recipe
consumer checks as a Wii U result. Original archives, binary recipes and
complete current Setup source/build dependencies must be supplied separately.
