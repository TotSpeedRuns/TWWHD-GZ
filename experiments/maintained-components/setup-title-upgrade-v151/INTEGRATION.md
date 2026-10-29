# Exact integration seams

Use these edits only with a separately authenticated Setup source that contains
the shown anchors. The reviewed v150 Personal and Public inputs differ in their
RPX/revision binding; this title overlay preserves those edition bytes. Normal
v151 RPX/version binding is a separate step. The snapshot is not a replacement
for the current Setup source or its release gates.

Copy the two files from `source/` beside that Setup's `main.cpp`. The adapter
uses the existing `Bytes`, `workspace`, `cancelRequested`, `fileHash`-compatible
SHA-256 implementation, `readFile`, restricted SD `writeFile`, `logPhase`,
`patchTitleArchive`, `gzcopy::DirectFile`, filesystem and
`flushPracticeVolume`. It adds no SDK call. Include order matters.

Apply the following five edits exactly once, preserving all other bytes.

1. Immediately after:

   ```cpp
   #include "fast_copy.hpp"
   ```

   add:

   ```cpp
   #include "title_upgrade.hpp"
   ```

2. Replace the title constant:

   ```cpp
   static constexpr const char* titleHash="718f8650532ff94d82db347bb9185858fd39b8492a9a91b6f3beb5108f1df819";
   ```

   with:

   ```cpp
   static constexpr const char* titleHash="029987c7d8d952aa284fcf345c5302b957d15dc9dc56c18927cc02074fab5510";
   ```

3. Immediately before the original `ownedIdentity` definition:

   ```cpp
   static bool ownedIdentity(const std::string& path,bool allowKnownPrior=false){
   ```

   add:

   ```cpp
   #include "title_upgrade_setup.inc"
   ```

   Its forward declaration matches the existing `flushPracticeVolume` definition
   later in the file. Do not replace that implementation.

4. Within that original `ownedIdentity` definition, replace only its final title
   expression:

   ```cpp
   &&fileHash(path+"/content/Common/Layout/Title_00.szs")==titleHash;
   ```

   with:

   ```cpp
   &&installedTitleIdentity(path,allowKnownPrior);
   ```

   Preserve the entire original title-path suffix, RPX allowlist and icon
   checks. The new helper admits the exact prior title only when
   `allowKnownPrior` is true. A missing live title requires its exact prior
   backup and an absent or exact selected stage. Strict final identity accepts
   only the selected live title.

5. Immediately before the original installed-RPX update branch:

   ```cpp
   if(installed&&fileHash(destination+"/code/cking.rpx")!=practiceHash){
   ```

   add:

   ```cpp
   if(installed) upgradeInstalledTitle(destination,original,sd+"/wiiu/apps/GZSetup/title.gzt",destVolume,base);
   ```

   This must remain after the existing original EUR v0 metadata and original
   RPX checks, and before RPX repair. The existing fresh-install
   `patchTitleArchive` call, final `ownedIdentity(destination)` checks, content
   verification, flush and completion-receipt ordering stay in place.

## Separately supplied original-dependent resource

The runtime SD resource `title.gzt` must be the separately reviewed original-only
recipe with SHA-256
`d8bcb9e4987adaf1771a7bf7be3739be37bef14fbb94c5f00566c50ff9ba93e6`.
It accepts only the user's original EUR v0 title archive with SHA-256
`06aaa05d996605d379b04e7ab1dfb9592d8d739c7c13e97bcd23049d33f29c5a`
and produces the selected `029987c7...` archive. No binary resource is supplied
by this source snapshot. Do not patch the installed legacy archive with an
original-only recipe.

The adapter builds on private SD staging, checks the selected recipe and
original, then writes the verified result to these fixed names under the
checked practice title's existing `content/Common/Layout/` folder:

- Live: `Title_00.szs`
- Stage: `Title_00-v151-029987c7d8d9.upload`
- Retained backup: `Title_00-before-v151-718f8650532f.szs.bak`

There is no scanning, unknown-file deletion, generic rename overwrite, source
game write or save write. A failed or inconsistent state is retained. Reusing
the fixed names for a different title revision requires a separately reviewed
source change; changing an app label alone does not bind different resources.

The complete native adapter and integrated installer still require their normal
toolchain, source/layout review and scoped platform tests. The 142 portable
tests validate the transaction header, not the native filesystem adapter or
reboot persistence.
