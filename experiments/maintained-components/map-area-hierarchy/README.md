# Map area folders

This authored experimental module groups the practice warp map by area while
keeping local destinations directly accessible. Headstone has Island and
Submarine entries plus an Earth Temple folder. Gale Isle keeps its two local
entries plus Wind Temple. The Tower of the Gods sector keeps its own warps
directly accessible and adds separate Hyrule and Ganon's Tower folders.

Dragon Roost, Forest Haven, Windfall, Outset and Forsaken Fortress also group
their large mixed lists by related area. All 49 sectors and all 218 existing
leaf IDs, labels and destination tuples are preserved. Back restores the
selected parent folder row. Favorites, text-browser groups and the existing
803 route payloads are unchanged.

`hierarchy.s` is the new module. `runtime-seams.json` contains only the eight
exact authored edits to the existing map controller/draw code, as readable
name/before/after records. It does not append another copy of the module.
`hierarchy-facts.json` is a readable functional inventory.
Read [the integration contract](RUNTIME_CONTRACT.md) before linking.

The private R824 image passed an actual controller-only Cemu test: 49 sector
selections, 12 folder enter/Back paths and four required A/B/A cycles. All 60
retained text-submission observations matched the expected titles and visible
labels. The 968 recorded host stores used only the three compiled controller
input spans; six ordinary/source/disposable save hashes stayed unchanged.
Independent qualification checked that terminal result with 2,767 checks.

This is scoped Cemu navigation and retained text-submission evidence. The test
did not select leaf warps, capture screenshots, verify TV/GamePad pixels or
test Wii U hardware. Retained text is not an atomic rendered-frame proof.

This source proposal does not change the shipped v145 release. The complete
reproducible public runtime remains v126. Its older template is not the exact
baseline required by these seams. Full runtime assembly, board artwork, game
files, objects, raw observations and process/profile details are excluded.

Check the standalone source facts without a compiler or game:

```sh
python validate_facts.py
```

That command checks table consistency and source pins, not native behavior.
The scoped evidence and source identities are recorded in `scope.json`.
Authored project code is MIT licensed under the repository's root license.
