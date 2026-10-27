# Map area folders

This authored experimental module groups the practice warp map by area while
keeping local destinations directly accessible. Headstone has Island and
Submarine entries plus an Earth Temple folder. Gale Isle keeps its two local
entries plus Wind Temple. The Tower of the Gods sector keeps its own warps
directly accessible and adds separate Hyrule and Ganon's Tower folders.

Dragon Roost, Forest Haven, Windfall and Outset also group their large mixed
lists by related area. Forsaken Fortress now has separate FF1, FF2 and FF3
folders, plus a direct `Exterior (FF2/FF3)` entry. All 49 sectors and all 218
existing leaf IDs and destination tuples are preserved. The exterior text is
a hierarchy-only display alias; its canonical label remains `Island`. Back
restores the selected parent folder row. Favorites, text-browser groups and the existing
803 route payloads are unchanged.

`hierarchy.s` is the new module. `runtime-seams.json` contains only the eight
exact authored edits to the existing map controller/draw code, as readable
name/before/after records. It does not append another copy of the module.
`hierarchy-facts.json` is a readable functional inventory.
Read [the integration contract](RUNTIME_CONTRACT.md) before linking.

The first-visit interior is `majroom`; the second and third variants are
`ma2room` and `ma3room`. Their original tuples stay distinct. FF1 uses folder
ID 0, FF2 uses 12 and FF3 uses 13; the other eleven folder IDs are unchanged.
The direct exterior keeps the original `sea`, room 1 tuple shared by the
second/third visit. The module does not invent or set story or inventory flags.

The FF2 folder retains the existing `EMS CS Part 2` shortcut (ID 113): `ADMumi`,
spawn 0xF4, room 0, layer 0x0B. This is a related Valoo-rescue cutscene shortcut,
not an assertion that ADMumi is an FF2 map. Visit names are corroborated by
[wwrando stage names](https://github.com/LagoLunatic/wwrando/blob/master/data/stage_names.txt#L175-L192)
and [noclip's Fortress scenes](https://github.com/magcius/noclip.website/blob/main/src/ZeldaWindWaker/Main.ts#L1100-L1108).
The [Valoo-rescue scene mapping](https://github.com/magcius/noclip.website/blob/main/src/ZeldaWindWaker/Main.ts#L1026-L1030)
identifies ADMumi layer 11. Owned HD metadata independently corroborated the
distinct interiors and the `runaway_majuto` event. No game resources are included.
Public source links track branches; no immutable revision is implied.

The private R829 image passed an actual controller-only Cemu test of all 49
sectors, all 14 folder enter/Back paths and seven required A/B/A repeat cycles,
including FF1, FF2 and FF3. Every folder Back restored its parent cursor; the
original empty-sector refusal remained. All 62/62 retained text-submission
observations matched the expected titles and labels. The 1,040 recorded host
stores used only the three compiled controller input spans. All six source and
disposable save-context file hashes remained unchanged through cleanup. The
test took 114.312 seconds; independent qualification passed 3,060 checks.

This native result belongs to the 14-folder derivative and joins its separate
source, detached emitted PPC and packed-main checks. No warp leaf was selected.
The ordinary existing-file context remained at sea room 44. See `scope.json`
for exact image, result and independent qualification identities.

No leaf warp, screenshot, TV/GamePad pixel or Wii U hardware acceptance is
claimed here. Retained text submissions are not an atomic rendered-frame proof.

This source proposal does not change the shipped v145 release. The complete
reproducible public runtime remains v126. Its older template is not the exact
baseline required by these seams. Full runtime assembly, board artwork, game
files, objects, raw observations and process/profile details are excluded.

Check the standalone source facts without a compiler or game:

```sh
python validate_facts.py
```

That command resolves every local title and folder-row label in the assembly
and compares the actual strings with the facts. It also checks all leaf IDs,
external label-symbol identities and source pins. External string definitions
remain an integration dependency; this command does not test native behavior.
The scoped evidence and source identities are recorded in `scope.json`.
Authored project code is MIT licensed under the repository's root license.
