# Development and source publication

Run `python -m unittest discover -s tests` and `python tools/audit_source.py` before
committing. These portable tests validate publication/asset boundaries; they do
not replace native engine or Wii U tests.

The maintainer's completed build directory contains `patch_wwhdgz.asm`, the trainer
and Morth sources, and `native-build/` with its matching ELF, RPX and build proof.
After the revision's runtime checks pass, its release workflow invokes:

```sh
python tools/publish.py --candidate /path/to/completed-build --revision v126 --push
```

This updates the source template and asset hashes, audits the allowed tree, runs
portable tests, commits changed source and pushes using normal Git credentials.
It never publishes the candidate directory itself, embeds credentials, force
pushes, or uploads game files. With no changes it does not create a new commit.
Use `python tools/publish.py --push` for reviewed changes already made here.

For the maintainer checkout, `git config gz.autopublish true` enables the same
audit/test/commit/push step after a successful `build.py` build. This setting is
local and is not inherited by clones. Set it to `false` to disable automatic
publication. It does not create a background watcher or scheduled task.

Adding another kind of embedded artwork requires updating the exporter and
reviewing the omission manifest before publication. Private experiments should
remain clearly labeled and must not silently replace a shipped runtime.
