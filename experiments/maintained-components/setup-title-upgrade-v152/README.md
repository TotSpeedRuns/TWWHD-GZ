# Installed-title transaction adapter (v152)

This is the exact authored title transaction/header/test snapshot used to
package v152 Personal and Public Setup. The complete reproducible integrated
public builder remains v126; these files do not build the current installer
alone. No title archive, Tingle artwork, font, original-dependent binary recipe,
game executable, SDK or private capture is included.

The adapter recognizes only the exact known prior GZ title hashes and the
selected replacement. It creates a verified same-folder stage through existing
Setup APIs and retains the previous title in the fixed v152 backup. Unknown
files refuse. The v151 backup path is preserved. Renames, rollback, cancellation
and strict final identity remain governed by the actual shared header.

ROOT compiled and ran the exported actual-header harness: 142 cases passed.
Both Setup editions also passed 10 checks of the actual C++ title recipe
consumer against the user's locally owned original. Those checks include full
output equality and malformed input refusal. No binary input is distributed.
The adapter was compiled in both native Setup editions. These host/compiler
checks do not qualify Wii U filesystem failure behavior or resolve the earlier
final-Repair freeze.

The generated v152 title changes exactly one archive byte from v151's final
version digit 1 to 2. All other bytes, including fonts, materials, size, placement
and Tingle content, are identical. The actual v151 native visual qualification
therefore supplies inherited layout parity. **No fresh v152 native visual run
is claimed.** Hashes and the original-dependent recipe identity are metadata
only in this export.

In a Visual Studio developer shell:

```powershell
cl /nologo /std:c++20 /EHsc /W4 /WX test_title_upgrade.cpp /Fe:title_upgrade_test.exe
./title_upgrade_test.exe
```

`title_recipe_driver.cpp` additionally needs the matching current Setup
`title_recipe.cpp`, setup types, endian/SHA implementations and locally owned
original/recipe/expected target arguments. It is not standalone without them.
See [integration notes](INTEGRATION.md) and `source-pins.json`.
