# TWWHD-GZ

Practice tools for The Legend of Zelda: The Wind Waker HD (European base game,
version 0), with route checkpoints, warps, inventory tools and trick trainers.

Created by Tot_sr (Aydin), with contributions documented in the source.

The complete reproducible runtime export in this repository is **v126**. Newer
reviewed source is published under `experiments/maintained-components/`; that is
a partial snapshot including the reviewed post-v140 Forest Water and Inventory
UI corrections, older save-state components through R703, and isolated
post-v140 restore-performance source. It does not reproduce the complete current release. See [source status](docs/SOURCE_STATUS.md).

The native patcher and Wii U GZ Setup source are also included. **Save states
remain under development.** Actor recreation, resource ownership, transitions
and durable slots still have gaps; this is not a claim of full-game coverage.

## Build the native runtime

You need Python 3.10+, Pillow, devkitPPC, your own unmodified EUR v0 `code/cking.rpx`,
and the practice RPX matching this source revision for its local-only artwork.
No original game executable, artwork, saves, keys or SDK binaries are distributed
here. The accepted hashes are recorded in `runtime/local-assets.json`.

```sh
python -m pip install -r requirements.txt
python tools/local_assets.py --practice-rpx /path/to/practice/code/cking.rpx
python build.py --original-rpx /path/to/original/code/cking.rpx --toolchain /path/to/devkitPPC/bin
```

Outputs go into a new `build/` directory; the original executable is read only.
The source export is rebuilt with devkitPPC and compared against the matching
local candidate. See [build status](docs/BUILD.md).

## Repository contents

- `runtime/`: v126 assembly template, trainer and captured Morth setup code.
- `patcher/`: local RPX and extracted-folder patching code.
- `creator/`: Memfile Creator 0.6 GUI, read-only Cemu/Wii U adapters and capture core.
- `setup/`: Wii U installer source, vendor sources and notices.
- `experiments/save-states/`: unfinished integration components, separate from v125.
- `experiments/maintained-components/`: newer reviewed C and assembly components;
  not a complete current-release build export.
- `tools/`: asset separation, build conversion, source audit and publishing tools.

## Source updates

Completed revisions use the [publication command](CONTRIBUTING.md) to refresh this
curated tree, run its checks, commit and push to this repository. This happens as
part of preparing a revision; there is no timed task or scheduled chat message.
Game assets, saves, keys, local paths and diagnostics stay outside publication.

Original project code is MIT licensed. Third-party notices and license scopes are
listed in [NOTICE.md](NOTICE.md). This is an independent fan practice project.
