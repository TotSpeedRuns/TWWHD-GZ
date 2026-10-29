# Experimental v151 installed-title upgrade

This authored source snapshot adds a checked title-layout upgrade to the
existing GZ Setup repair path. It accepts the exact prior GZ title archive,
builds the selected replacement from the user's original EUR v0 archive, and
retains the prior title in a same-volume backup. Unknown files are refused.
Final identity still requires the exact selected replacement.

The complete reproducible integrated export remains v126. These files are a
partial maintained source snapshot, not a standalone current installer build.
They do not include a title archive, original-dependent binary recipe, artwork,
game executable, SDK, key, save, private runtime capture, or diagnostic log.

## Source and validation

- `source/title_upgrade.hpp`: the checked file-state transaction.
- `source/title_upgrade_setup.inc`: its adapter to the existing Setup file,
  original-only recipe, SHA-256 and volume-flush APIs.
- `test_title_upgrade.cpp`: portable tests including the actual transaction
  header, rather than a separate implementation of its policy.
- [Integration instructions](INTEGRATION.md): the five exact source seams and
  dependencies needed in a separately reviewed current Setup build.
- `source-pins.json`: portable source and private-proof fingerprints only.

The root compiled and ran the actual-header tests successfully: **142 cases**.
They cover all 64 live/stage/backup file-kind tuples, strict final identity,
staging and flush failures, cancellation, failures before and after either
rename, failed rollback, and a subsequent checked retry. The host qualification
fingerprint is
`c55c6734aa3062a591c1859d54bd8fcfd7e4f0cbb6064a9fb091514699f859cf`.
These policy cases do not exercise Wii U filesystem failure behavior.

From this folder, a Visual Studio developer shell can run:

```powershell
cl /nologo /std:c++20 /EHsc /W4 /WX test_title_upgrade.cpp /Fe:title_upgrade_test.exe
./title_upgrade_test.exe
```

The transaction writes a verified same-folder stage through the existing
`gzcopy::DirectFile`, flushes and closes it, then checks SHA-256. It renames the
old live file to a retained backup and the stage to the live file. If promotion
fails in the exact recognized intermediate state, it attempts and checks the
reverse rename. A failed rollback retains the known files for checked retry.
The backup is never automatically deleted.

Cancellation is checked before staging and before replacement. The bounded
rename/rollback span completes its small-file identity checks without a
cancellation callback. The helper never issues a completion receipt; existing
final RPX, icon, title, registration, content and volume checks remain required.

There is no claim of an atomic RPX/title pair update, generic recovery after
power loss, or acceptance of partial/unknown files. The new title transaction
has no Wii U qualification in this snapshot. It does **not** establish a fix for
the unresolved v150 final-repair freeze.

## v151 scope

The title change is separate from the previously published
[single-action quiet-item Restore continuation](../single-restore-continuation/README.md).
That experiment has finite Cemu HEART and green-control passes. It does not
qualify a mixed cohort, active effects, saved smoke, Wii U, or full-game save
states. Neither those runtime results nor the title's separate Cemu visual
qualification supply native evidence for this installer transaction.

This proposal does not update the reproducible-build baseline or assert that
the complete v151 runtime or installer has been published here.
